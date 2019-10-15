# owo whats this?
This just generates a big database of PP values with specific mods / accuracy, relatively quickly.

[Example of use](https://docs.google.com/spreadsheets/d/1vl2Fjzjy31ca_ylHZ3wwB5OjYJs1BZaJnZUkE0Rjc9Q/edit?usp=sharing)

# Requirements
- MySQL connector for C
- gcc, clang, or other C compiler

# Compiling
My compile line is as follows:
```
gcc danielmagical.c `mysql_config --cflags --libs` -o danielmagical.out -Wall -O3
```

# Usage
./danielmagical.out [OPTIONS]

Options can be used with the -- prefix, here are all options with examples:

--mods hddtrxhr
Sets the mods.

--wipe-db
Wipes any data previously in the database.

--acc 98.5
Sets the accuracy to 98.5% (float).

--min-pp 800
Sets the minimum PP value to be added to the database as 800 (float).

--max-pp 3000
Sets the maximum PP value to be added to the database as 3000 (float)

**Example of use:**
```
./danielmagical.out --mods ezdt --wipe-db --acc 96.35 --min-pp 200 --max-pp 500
```