// A Simple (and pretty beginner-level coded) map/pp finder made by cmyui as (basically) his first project in C.
#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>

#include </mnt/d/Development/Misc Tools/gen_table/oppai.h>

// Console colours
#define KNRM      "\x1B[0m"
#define KRED      "\x1B[31m"
#define KGRN      "\x1B[32m"
#define KYEL      "\x1B[33m"
#define KBLU      "\x1B[34m"
#define KMAG      "\x1B[35m"
#define KCYN      "\x1B[36m"
#define KGRY      "\x1B[37m"
#define KRESET    "\033[0m"
#define NULL_TERM '\0'

#define LogError(x) printf(KRED "ERR: " KRESET #x "\n")

char BEATMAP_FOLDER[sizeof("/mnt/d/Development/Misc Tools/gen_table/maps/1234567.osu")] = { '\0' },
     SQL[sizeof("INSERT IGNORE INTO pp_table(id,beatmap_id,pp,star_rating,pp_ratio)VALUES(NULL,1234567,1234.123,12.12345,123);")] = { '\0' };

enum Mods
{
  //noMod             = 0,
	NoFail            = 1 << 0,
	Easy              = 1 << 1,
  //NoVideo           = 1 << 2,
	Hidden            = 1 << 3,
	HardRock          = 1 << 4,
  //SuddenDeath       = 1 << 5,
	DoubleTime        = 1 << 6,
	Relax             = 1 << 7,
	HalfTime          = 1 << 8,
	Nightcore         = 1 << 9,
	Flashlight        = 1 << 10,
  //Autoplay          = 1 << 11,
	SpunOut           = 1 << 12,
  //Relax2            = 1 << 13,
  //Perfect           = 1 << 14,
  //Key4              = 1 << 15,
  //Key5              = 1 << 16,
  //Key6              = 1 << 17,
  //Key7              = 1 << 18,
  //Key8              = 1 << 19,
  //FadeIn            = 1 << 20,
  //Random            = 1 << 21,
  //Cinema            = 1 << 22,
  //Target            = 1 << 23,
  //Key9              = 1 << 24,
  //KeyCoop           = 1 << 25,
  //Key1              = 1 << 26,
  //Key3              = 1 << 27,
  //Key2              = 1 << 28,
  //LastMod           = 1 << 29,
  //KeyMod            = Key1 | Key2 | Key3 | Key4 | Key5 | Key6 | Key7 | Key8 | Key9 | KeyCoop,
  //FreeModAllowed    = NoFail | Easy | Hidden | HardRock | SuddenDeath | Flashlight | FadeIn | Relax | Relax2 | SpunOut | KeyMod,
  //ScoreIncreaseMods = Hidden | HardRock | DoubleTime | Flashlight | FadeIn,
  //TimeAltering      = DoubleTime | HalfTime | Nightcore
};

unsigned char FileExists(const char* BeatmapPath)
{
	FILE *f = fopen(BeatmapPath, "r");
    if (!f) return 1;

    fseek(f, 0, SEEK_END);
    const int Size = ftell(f);

    fclose(f);

    if (Size < 100) return 1;
    return 0;
}

int main(int argc, char *argv[])
{
    char sql_user[32], sql_pass[32], sql_db[32], sql_server[32];
    { // Config
        const char delim[2] = ":";
        FILE *f = fopen("config.ini", "r");

        if (!f)
        {
            LogError(No config file could be found!);
            return 1;
        }

        char line[64];

        while (fgets(line, sizeof(line), f))
        {
            { // trim newline off end
                size_t len = strlen(line);
                if (len > 0 && line[len-1] == '\n')
                    line[--len] = '\0';
            }

            char *key = strtok(line, delim);

            if      (!strcmp("sql_user",   key)) strncpy(sql_user,   strtok(NULL, delim), sizeof(sql_user));
            else if (!strcmp("sql_pass",   key)) strncpy(sql_pass,   strtok(NULL, delim), sizeof(sql_pass));
            else if (!strcmp("sql_db",     key)) strncpy(sql_db,     strtok(NULL, delim), sizeof(sql_db));
            else if (!strcmp("sql_server", key)) strncpy(sql_server, strtok(NULL, delim), sizeof(sql_server));
            else printf(KYEL "Unknown config key '%32s'" KRESET "\n", key);
        } fclose(f);
    }

    // Default settings.
    int              mods = Relax | HardRock | Hidden | DoubleTime;
    unsigned char wipe_db = 0;
    float          MAX_PP = 2750.f,
                   MIN_PP = 1000.f,
                 accuracy = 100.f;

    { // Launch flags.
        unsigned char inc = 0;
        for (int i = 0; i < argc; i++)
        {
            if (inc)
            {
                inc = 0;
                continue;
            }

            if (i  > argc - 2)
            {
                LogError(Invalid launch flags.);
                break;
            }

            // Specify specific mod(s).
            if (!strcmp("--mods", argv[i]))
            {
                mods = 0;
                char *mods_ascii = argv[i + 1];
                const int Size = strlen(mods_ascii);

                char current_mod[3];
                for (int i = 0; i < Size; i++, i++)
                {
                    sscanf(mods_ascii, "%2s", current_mod);

                    if (strlen(current_mod) != 2)
                        break;

                    if      (!strcmp("nf", current_mod)) mods |= NoFail;
                    else if (!strcmp("ez", current_mod)) mods |= Easy;
                    else if (!strcmp("hd", current_mod)) mods |= Hidden;
                    else if (!strcmp("hr", current_mod)) mods |= HardRock;
                    else if (!strcmp("dt", current_mod)) mods |= DoubleTime;
                    else if (!strcmp("rx", current_mod)) mods |= Relax;
                    else if (!strcmp("ht", current_mod)) mods |= HalfTime;
                    else if (!strcmp("nc", current_mod)) mods |= Nightcore;
                    else if (!strcmp("fl", current_mod)) mods |= Flashlight;
                    else if (!strcmp("so", current_mod)) mods |= SpunOut;
                    else printf(KYEL "Invalid mod '%s'" KRESET "\n", current_mod);

                    mods_ascii += 2;
                }

                inc = 1;
            }

            // Specify specific accuracy.
            else if (!strcmp("--acc", argv[i]))
            {
                sscanf(argv[i + 1], "%f", &accuracy);
                inc = 1;
            }

            // Specify to wipe DB.
            else if (!strcmp("--wipe-db", argv[i]))
            {
                wipe_db = 1;
            }

            // Specify specific max-pp.
            else if (!strcmp("--max-pp", argv[i]))
            {
                sscanf(argv[i + 1], "%f", &MAX_PP);
                inc = 1;
            }

            // Specify specific min-pp.
            else if (!strcmp("--min-pp", argv[i]))
            {
                sscanf(argv[i + 1], "%f", &MIN_PP);
                inc = 1;
            }
        }
    }

    if (MAX_PP < MIN_PP)
    {
        LogError(Max PP must be higher than Min PP!);
        return 1;
    }

    printf(KYEL "Settings:\n" KCYN "Mods: %i\nAccuracy: %.2f\nMax PP: %.2f\nMin PP: %.2f" KRESET "\n\n", mods, accuracy, MAX_PP, MIN_PP);

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, sql_server, sql_user, sql_pass, sql_db, 0, NULL, 0))
    {
        LogError(mysql_error(conn));
        return 1;
    }

    if (wipe_db)
    {
        if (mysql_query(conn, "TRUNCATE TABLE pp_table;"))
        {
            LogError(mysql_error(conn));
            return 1;
        }
        printf(KGRN "Wiped database." KRESET "\n");
    }

    if (mysql_query(conn, "SELECT beatmap_id FROM beatmaps WHERE ranked IN (-2, 2) ORDER BY beatmap_id ASC;"))
    {
        LogError(mysql_error(conn));
        return 1;
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
            LogError(Failed to load ezpp.);
            return 1;
        }

        // oppai settings.
        ezpp_set_mods             (ez, mods);
        ezpp_set_accuracy_percent (ez, accuracy);
        /*
        ezpp_set_nmiss            (ez, 0);
        ezpp_set_mode             (ez, 0);
        ezpp_set_combo            (ez, 0);
        */

        snprintf(BEATMAP_FOLDER, sizeof(BEATMAP_FOLDER), "/mnt/d/Development/Misc Tools/gen_table/maps/%7i.osu", BeatmapArray[i]);

        if (FileExists(BEATMAP_FOLDER))
        {
            ezpp_free(ez);
            continue;
        }

        // Init oppai's ezpp with the map's path.
        ezpp(ez, BEATMAP_FOLDER);

        if (ez->pp < MIN_PP || ez->pp > MAX_PP || is_nan(ez->pp))
        {
            ezpp_free(ez);
            continue;
        }

        { // Accepted val
            int pp_ratio = (int)((ez->speed_pp / (ez->aim_pp + ez->speed_pp + ez->acc_pp)) * 100); // In Akatsuki pp, ez->speed_pp isnt part of ez->pp.

            snprintf(
                SQL,
                sizeof(SQL),
                "INSERT IGNORE INTO pp_table(id,beatmap_id,pp,star_rating,pp_ratio)VALUES(NULL,%7i,%.3f,%.5f,%3i);",
                BeatmapArray[i],
                ez->pp,
                ez->stars,
                pp_ratio
            );

            if (mysql_query(conn, SQL))
            {
                LogError(mysql_error(conn));
                return 1;
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
    return 0;
}
