// A Simple (and pretty beginner-level coded) map/pp finder made by cmyui as (basically) his first project in C.
//#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>
//#include <string.h>

#include </mnt/d/Development/Misc Tools/gen_table/oppai.h>

// Console colours
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

#define LogError(x) printf(KRED "ERROR CODE "#x KRESET "\n")

// TODO: config
#define DEBUG 0

// SQL
#define SQL_SERVER "localhost"
#define SQL_USER "root"
#define SQL_PASSWD "no"
#define SQL_DB "ripple"

// Path to beatmaps folder
#define __BeatmapPath "/mnt/d/Development/Misc Tools/gen_table/maps/"

const size_t path_max_size = sizeof(__BeatmapPath) + 11; // max size of beatmap path
const size_t sql_max_size = sizeof("INSERT IGNORE INTO pp_table(id,beatmap_id,pp,star_rating,pp_ratio)VALUES(NULL,1234567,1234.123,12.12345,123);"); // max size of sql insert to pp_table
char MapPath[sizeof(__BeatmapPath) + 11] = { '\0' };
char SQL[sizeof("INSERT IGNORE INTO pp_table(id,beatmap_id,pp,star_rating,pp_ratio)VALUES(NULL,1234567,1234.123,12.12345,123);")] = { '\0' };

// Settings
#define MAX_PP 2750.f
#define MIN_PP 1000.f

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

unsigned char FileExists(const char* BeatmapPath)
{
	FILE *f = fopen(BeatmapPath, "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    const int Size = ftell(f);

    fclose(f);

    if (Size < 100) return 0;
    return 1;
}

int main(int argc, char *argv[])
{
    // Default mods
    int mods = Relax | HardRock | Hidden | DoubleTime;
    float accuracy = 100.f;

    // TODO: actually properly do these lol.
    // Also, I can add quite a bit of cool flag things here!
    if (argc > 1) mods     = atoi(argv[1]);
    if (argc > 2) accuracy = atof(argv[2]);
    if (!mods || mods > 1073741824 || !accuracy || accuracy > 100.f)
    {
        printf("\n" KRED "Invalid custom settings (%i, %.2f%%)\n" KRESET "\n", mods, accuracy);
        return 0;
    }

    printf(KYEL "Settings:\n" KCYN "Mods: %i\nAccuracy: %.2f" KRESET "\n\n", mods, accuracy);

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, SQL_SERVER, SQL_USER, SQL_PASSWD, SQL_DB, 0, NULL, 0))
    {
        fprintf(stderr, KRED "%s" KRESET "\n", mysql_error(conn));
        return 0;
    }

    if (mysql_query(conn, "SELECT beatmap_id FROM beatmaps WHERE ranked IN (-2, 2) ORDER BY beatmap_id ASC"))
    {
        fprintf(stderr, KRED "%s" KRESET "\n", mysql_error(conn));
        return 0;
    }

    res = mysql_store_result(conn);

    unsigned long long row_count = mysql_num_rows(res);
    printf(KCYN "Found %llu beatmaps matching criteria. Calculating PP values.." KRESET "\n", row_count);

    // Build beatmap array
    int* BeatmapArray = calloc(row_count, sizeof(int));
    for (int i = 0; (row = mysql_fetch_row(res)) != NULL; i++)
        BeatmapArray[i] = atoi(row[0]);

    mysql_free_result(res);

    for (int i = 0; i <= row_count; i++)
        {
        ezpp_t ez = ezpp_new();

        if (!ez)
        {
            printf("\n" KRED "** Failed to load ezpp. **" KRESET "\n");
            return 0;
        }

        // oppai settings.
        ezpp_set_mods             (ez, mods);
        ezpp_set_accuracy_percent (ez, accuracy);
        /*
        ezpp_set_nmiss            (ez, 0);
        ezpp_set_mode             (ez, 0);
        ezpp_set_combo            (ez, 0);
        */

        snprintf(MapPath, path_max_size, "/mnt/d/Development/Misc Tools/gen_table/maps/%i.osu", BeatmapArray[i]);

        if (!FileExists(MapPath))
            continue;

        // Init oppai's ezpp with the map's path.
        ezpp(ez, MapPath);

#if DEBUG
        printf("PP: %.3f\n", ez->pp);
#endif

        if (ez->pp < MIN_PP || ez->pp > MAX_PP || is_nan(ez->pp))
            continue;

        { // Accepted val
            int pp_ratio = (int)((ez->speed_pp / (ez->aim_pp + ez->speed_pp + ez->acc_pp)) * 100); // In Akatsuki pp, ez->speed_pp isnt part of ez->pp.

            snprintf(
                SQL,
                sql_max_size,
                "INSERT IGNORE INTO pp_table(id,beatmap_id,pp,star_rating,pp_ratio)VALUES(NULL,%i,%.3f,%.5f,%i);",
                BeatmapArray[i],
                ez->pp,
                ez->stars,
                pp_ratio
            );

            if (mysql_query(conn, SQL))
            {
                fprintf(stderr, KRED "%s" KRESET "\n", mysql_error(conn));
                return 0;
            }

            printf(
                "\n" KGRN
                "Accepted value." KRESET "\n"
                "Beatmap ID: %i\n"
                "PP: %.3fpp\n"
                "Star Rating: %.5f*\n"
                "Stream pp: %i%%\n",
                BeatmapArray[i],
                ez->pp,
                ez->stars,
                pp_ratio
            );
        }

        ezpp_free(ez);
    }

    mysql_free_result(res);
    mysql_close(conn);

    printf("\n" KGRN "Closed successfully." KRESET "\n");
    return 1;
}
