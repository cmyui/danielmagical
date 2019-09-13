#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <string.h>

#include </mnt/d/Development/pp_98/oppai.h>
//#include <windows.h>

#define LINUX 1

// SQL
#define SQL_SERVER "localhost"
#define SQL_USER "root"
#define SQL_PASSWD "no"
#define SQL_DB "ripple"

#define __BeatmapPath "/mnt/d/Development/pp_98/maps/"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KGRY  "\x1B[37m"
#define KRESET "\033[0m"

#ifndef __cplusplus
typedef unsigned char bool;
#endif

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

void LogError(int x) { printf(KRED "ERROR CODE %i" KRESET "\n", x); }

void _SQL(MYSQL *conn, char Query[]) {
    if (mysql_query(conn, Query)) {
        fprintf(stderr, KRED "%s" KRESET "\n", mysql_error(conn));
        exit(1);
    }
}

bool GetFileSize(const char* BeatmapPath) {
	FILE *f = fopen(BeatmapPath, "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    const int Size = ftell(f);

    fclose(f);
    if (Size >= 100) return 1;
    return 0;
}

int main(int argc, char** argv) {
    MYSQL *conn;
    MYSQL *_conn; // TODO: remove
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, SQL_SERVER, SQL_USER, SQL_PASSWD, SQL_DB, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 1;
    }

    _conn = mysql_init(NULL);
    if (!mysql_real_connect(_conn, SQL_SERVER, SQL_USER, SQL_PASSWD, SQL_DB, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(_conn));
        return 1;
    }

    _SQL(conn, "SELECT COUNT(id) FROM beatmaps WHERE ranked = 2");
    res = mysql_use_result(conn);

    int row_count;
    while ((row = mysql_fetch_row(res)) != NULL)
        row_count = atoi(row[0]);


    int* BeatmapArray = calloc(row_count, sizeof(int));

    _SQL(conn, "SELECT beatmap_id FROM beatmaps WHERE ranked = 2 ORDER BY beatmap_id ASC");

    res = mysql_use_result(conn);
    int i = 1; // TODO: learn to for loop this
    while ((row = mysql_fetch_row(res)) != NULL) {
        BeatmapArray[i] = atoi(row[0]);
        i++;
    }

    char BeatmapID[12];
    char MapPath[strlen(__BeatmapPath) + 11];
    float PP, MapStars;

    for (i=0; i <= row_count; i++) {
        ezpp_t ez = ezpp_new();
        if (!ez) {
            printf("Failed to load ezpp.\n");
            return 1;
        }

        ezpp_set_mods(ez, Relax);
        //ezpp_set_nmiss(ez, 0);
        ezpp_set_accuracy_percent(ez, 99.f);
        //ezpp_set_combo(ez, sData.MaxCombo);
        //ezpp_set_mode(ez, 0);

        //printf("%i - %i\n", BeatmapArray[i], i);
        sprintf(BeatmapID, "%d", BeatmapArray[i]);

        strcat(MapPath, __BeatmapPath);
        strcat(MapPath, BeatmapID);
        strcat(MapPath, ".osu");

        if (!GetFileSize(MapPath)) {
            //printf(KRED "Failed %s" KRESET "\n", BeatmapID);
            strcpy(MapPath, "");
            continue;
        }

        // Init oppai's ezpp with the map's path.
        ezpp(ez, MapPath);
        strcpy(MapPath, "");
        PP = ezpp_pp(ez);
        MapStars = ezpp_stars(ez);

        //printf("%f\n", PP);

        if (!PP || PP < 750.f || PP > 4000.f || isnan(PP)) continue;//ignore stupid numbers

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
        gcvt(MapStars, 7, _SR);
        _SR[7] = '\0';
        strcat(SQLInsert, _SR);

        strcat(SQLInsert, ");");
        _SQL(_conn, SQLInsert);

        printf("Inserting accepted value: %fpp - %spp (%s*)\n", PP, _PP, _SR);
        strcpy(SQLInsert, "");

        PP = 0.f;
        MapStars = 0.f;

        ezpp_free(ez);
    }

    mysql_free_result(res);
    mysql_close(conn);
    mysql_close(_conn);
    return 0;
}
