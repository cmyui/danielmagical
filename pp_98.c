// A Simple (and pretty beginner-level coded) map/pp finder made by cmyui as (basically) his first project in C.
#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>

#include </mnt/d/Development/Misc Tools/pp_98/oppai.h>
//#include <windows.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KGRY  "\x1B[37m"
#define KRESET "\033[0m"

//#define DEBUG

// TODO: config
#define SQL_SERVER "localhost"
#define SQL_USER "root"
#define SQL_PASSWD "no"
#define SQL_DB "ripple"
#define __BeatmapPath "/mnt/d/Development/Misc Tools/pp_98/maps/"

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

#ifdef DEBUG
    void LogError(int x) { printf(KRED "ERROR CODE %i" KRESET "\n", x); }
#endif

unsigned char _SQL(MYSQL* conn, char Query[]) {
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

int main(int argc, char** argv) {

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

    if (!_SQL(conn, "SELECT beatmap_id FROM beatmaps WHERE ranked = 2 ORDER BY beatmap_id ASC"))
        return 0;

    res = mysql_store_result(conn);

    unsigned long long row_count = mysql_num_rows(res);
    printf(KCYN "Beatmaps found (database): %llu" KRESET "\n", row_count);

    // Build beatmap array
    int* BeatmapArray = calloc(row_count, sizeof(int));
    for (int i = 1; (row = mysql_fetch_row(res)) != NULL; i++)
        BeatmapArray[i] = atoi(row[0]);
    mysql_free_result(res);

    char BeatmapID[12];
    char MapPath[strlen(__BeatmapPath) + 11];
    float PP, MapStars;

    for (int i=0; i <= row_count; i++) {
        ezpp_t ez = ezpp_new();
        if (!ez) {
            printf(KRED "\n** Failed to load ezpp. **" KRESET "\n");
            return 0;
        }

        ezpp_set_mods(ez, Relax + HardRock + Hidden + DoubleTime);
        //ezpp_set_nmiss(ez, 0);
        ezpp_set_accuracy_percent(ez, 99.f);
        //ezpp_set_combo(ez, sData.MaxCombo);
        //ezpp_set_mode(ez, 0);

#ifdef DEBUG
        printf(KMAG "\nBeatmapArray[i]: %i\ni: %i" KRESET "\n", BeatmapArray[i], i);
#endif
        sprintf(BeatmapID, "%d", BeatmapArray[i]);

        strcat(MapPath, __BeatmapPath);
        strcat(MapPath, BeatmapID);
        strcat(MapPath, ".osu");

        if (!GetFileSize(MapPath)) {
#ifdef DEBUG
            printf(KRED "\nFailed %s" KRESET "\n", BeatmapID);
#endif
            strcpy(MapPath, "");
            continue;
        }

        // Init oppai's ezpp with the map's path.
        ezpp(ez, MapPath);
        strcpy(MapPath, "");
        PP = ezpp_pp(ez);
        MapStars = ezpp_stars(ez);

#ifdef DEBUG
        printf("%f\n", PP);
#endif

        if (!PP || PP < 1500.f || PP > 4000.f || is_nan(PP)) { // TODO: change to is_nan from oppai.h
#ifdef DEBUG
            printf(KRED "\nBAD Failed %s (%.2fpp)" KRESET "\n", BeatmapID, PP);
#endif
            continue;
        }

        //TODO: sql prepare statement?
        char SQLInsert[120];
        strcat(SQLInsert, "INSERT IGNORE INTO daniel_maps (id, beatmap_id, pp, star_rating) VALUES (NULL, ");
        strcat(SQLInsert, BeatmapID);

        strcat(SQLInsert, ", ");
        static char _PP[8];
        gcvt(PP, 7, _PP);
        _PP[7] = '\0';
        strcat(SQLInsert, _PP);

        strcat(SQLInsert, ", ");
        static char _SR[8];
        gcvt(MapStars, 8, _SR);
        _SR[7] = '\0';

        strcat(SQLInsert, _SR);

        strcat(SQLInsert, ");");
        if (!_SQL(conn, SQLInsert))
            return 0;

        printf(KGRN "\nInserting accepted value" KRESET "\nBeatmap ID: %s\nPP: %spp (%fpp)\nStar Rating: %s*\n", BeatmapID, _PP, PP, _SR);

        // TODO: find out how to actually do this?
        strcpy(SQLInsert, "");
        strcpy(_SR, "");
        strcpy(_PP, "");

        PP = 0.f;
        MapStars = 0.f;

        ezpp_free(ez);
    }

    mysql_free_result(res);
    mysql_close(conn);
    return 1;
}
