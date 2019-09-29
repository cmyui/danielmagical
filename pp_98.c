// A Simple (and pretty beginner-level coded) map/pp finder made by cmyui as (basically) his first project in C.
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>

#include </mnt/d/Development/Misc Tools/pp_98/oppai.h>

#define DEBUG 0

#define LogError(x) printf(KRED "ERROR CODE "#x KRESET "\n")

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KGRY  "\x1B[37m"
#define KRESET "\033[0m"
#define NULL_TERM '\0'

// TODO: config
#define SQL_SERVER "localhost"
#define SQL_USER "root"
#define SQL_PASSWD "no"
#define SQL_DB "ripple"
#define __BeatmapPath "/mnt/d/Development/Misc Tools/pp_98/maps/"
#define ACCURACY 98.f
#define MAX_PP 4000.f
#define MIN_PP 1500.f
#define SQLInsert_Max_Len 112

enum Mods
{
	noMod = 0,
	NoFail = 1 << 0,
	Easy = 1 << 1,
	//NoVideo              = 1 << 2,
	Hidden = 1 << 3,
	HardRock = 1 << 4,
	SuddenDeath = 1 << 5,
	DoubleTime = 1 << 6,
	Relax = 1 << 7,
	HalfTime = 1 << 8,
	Nightcore = 1 << 9,
	Flashlight = 1 << 10,
	Autoplay = 1 << 11,
	SpunOut = 1 << 12,
	Relax2 = 1 << 13,
	Perfect = 1 << 14,
	Key4 = 1 << 15,
	Key5 = 1 << 16,
	Key6 = 1 << 17,
	Key7 = 1 << 18,
	Key8 = 1 << 19,
	FadeIn = 1 << 20,
	Random = 1 << 21,
	Cinema = 1 << 22,
	Target = 1 << 23,
	Key9 = 1 << 24,
	KeyCoop = 1 << 25,
	Key1 = 1 << 26,
	Key3 = 1 << 27,
	Key2 = 1 << 28,
	LastMod = 1 << 29,
	KeyMod = Key1 | Key2 | Key3 | Key4 | Key5 | Key6 | Key7 | Key8 | Key9 | KeyCoop,
	FreeModAllowed = NoFail | Easy | Hidden | HardRock | SuddenDeath | Flashlight | FadeIn | Relax | Relax2 | SpunOut | KeyMod,
	ScoreIncreaseMods = Hidden | HardRock | DoubleTime | Flashlight | FadeIn,
	TimeAltering = DoubleTime | HalfTime | Nightcore
};

unsigned char _Query(MYSQL* conn, char Query[]) {
    if (mysql_query(conn, Query)) {
        fprintf(stderr, KRED "%s" KRESET "\n", mysql_error(conn));
        return 0;
    } return 1;
}

unsigned char GetFileSize(const char* BeatmapPath) {
	FILE *f = fopen(BeatmapPath, "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    const int Size = ftell(f);

    fclose(f);

    if (Size < 100) return 0;
    return 1;
}

int main(void) {

    /* Setup connection with MySQL */
    MYSQL *conn;
    MYSQL *_conn; // TODO: remove
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, SQL_SERVER, SQL_USER, SQL_PASSWD, SQL_DB, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 0;
    }

    if (!_Query(conn, "SELECT beatmap_id FROM beatmaps WHERE ranked IN (-2, 2) ORDER BY beatmap_id ASC")) return 0;

    res = mysql_store_result(conn);

    unsigned long long row_count = mysql_num_rows(res);
    printf(KCYN "Beatmaps found (database): %llu" KRESET "\n", row_count);

    // Build beatmap array
    int* BeatmapArray = calloc(row_count, sizeof(int));
    for (int i = 0; (row = mysql_fetch_row(res)) != NULL; i++)
        BeatmapArray[i] = atoi(row[0]);

    mysql_free_result(res);

    char BeatmapID[8];
    char MapPath[strlen(__BeatmapPath) + 11];// TODO: const size
    float PP, MapStars;

    for (int i = 0; i <= row_count; i++) {
        ezpp_t ez = ezpp_new();
        if (!ez) {
            printf(KRED "\n** Failed to load ezpp. **" KRESET "\n");
            return 0;
        }

        ezpp_set_mods(ez, Relax + HardRock + Hidden + DoubleTime);
        ezpp_set_nmiss(ez, 0);
        ezpp_set_accuracy_percent(ez, ACCURACY);
        //ezpp_set_combo(ez, sData.MaxCombo);
        ezpp_set_mode(ez, 0);

#if DEBUG
        printf(KMAG "\nBeatmapArray[i]: %i\ni: %i" KRESET "\n", BeatmapArray[i], i);
#endif
        sprintf(BeatmapID, "%d", BeatmapArray[i]);
        BeatmapID[7] = NULL_TERM;

        strcat(MapPath, __BeatmapPath);
        strcat(MapPath, BeatmapID);
        strcat(MapPath, ".osu");

        if (!GetFileSize(MapPath)) {
#if DEBUG
            printf(KRED "\nFailed %s" KRESET "\n", BeatmapID);
#endif
            strcpy(MapPath, "\0");
            continue;
        }

        // Init oppai's ezpp with the map's path.
        ezpp(ez, MapPath);
        strcpy(MapPath, "\0");

        /* ez->pp and ez->stars?
        PP = ezpp_pp(ez);
        MapStars = ezpp_stars(ez);
        */

#if DEBUG
        printf("PP: %.2f\n", ez->pp);
#endif

        if (ez->pp < MIN_PP || ez->pp > MAX_PP || is_nan(ez->pp)) { // oppai occasionally returns a nan value.. very cool kanye
#if DEBUG
            printf(KRED "\nBAD Failed %s (%.2fpp)" KRESET "\n", BeatmapID, ez->pp);
#endif
            continue;
        }

        //TODO: sql prepare statement?
        char SQLInsert[SQLInsert_Max_Len] = { '\0' };
        strcat(SQLInsert, "INSERT IGNORE INTO daniel_maps(id,beatmap_id,pp,star_rating,pp_ratio)VALUES(NULL,");

        strcat(SQLInsert, BeatmapID);
        strcat(SQLInsert, ",");

        static char
            _PP[8] = { '\0' },
            _SR[8] = { '\0' }
        ;

        // PP
        gcvt(ez->pp, 7, _PP);
        _PP[7] = NULL_TERM;
        strcat(SQLInsert, _PP);
        strcat(SQLInsert, ",");

        // SR
        gcvt(ez->stars, 8, _SR);
        _SR[7] = NULL_TERM;
        strcat(SQLInsert, _SR);
        strcat(SQLInsert, ",");

        char pp_ratio[5];
        gcvt((int)((ez->speed_pp / ez->pp) * 100), 4, pp_ratio);
        pp_ratio[4] = NULL_TERM;
        strcat(SQLInsert, pp_ratio);
        strcat(SQLInsert, ");");

        if (!_Query(conn, SQLInsert)) return 0;

        printf(
            KGRN "\n"
            "Inserting accepted value" KRESET "\n"
            "Beatmap ID: %s\n"
            "PP: %spp (%fpp)\n"
            "Star Rating: %s*\n"
            "Stream pp: %s%%\n",
            BeatmapID,
            _PP,
            ez->pp,
            _SR,
            pp_ratio
        );

#if DEBUG
        printf(
            "SQLInsert:\n"
            "len: %zu\n"
            "str: \"%s\"\n",
            strlen(SQLInsert),
            SQLInsert
        );
#endif

        if (strlen(SQLInsert) > SQLInsert_Max_Len)
        {
            printf(
                KRED "\n\n"
                "SQLInsert length too large for variable, possibly memory leaking!\n"
                "Maxlen: %i\n"
                "Len: %zu\n"
                "String: \"%s\"" KRESET "\n",
                SQLInsert_Max_Len,
                strlen(SQLInsert),
                SQLInsert
            );
            return 0;
        }

        // TODO: find out how to actually do this?
        //strcpy(SQLInsert, "\0");
        //strcpy(_SR, "\0");
        //strcpy(_PP, "\0");

        //PP = 0.f;
        //MapStars = 0.f;

        ezpp_free(ez);
    }

    mysql_free_result(res);
    mysql_close(conn);

    printf("\n" KGRN "Closed successfully." KRESET "\n");
    return 1;
}
