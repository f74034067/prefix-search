#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tst.h"

/** constants insert, delete, max word(s) & stack nodes */
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };
#define REF INS
#define CPY DEL
#define poolSize 10000000

/* timing helper function */
static double tvgetf(void)
{
    struct timespec ts;
    double sec;

    clock_gettime(CLOCK_REALTIME, &ts);
    sec = ts.tv_nsec;
    sec /= 1e9;
    sec += ts.tv_sec;

    return sec;
}

/* simple trim '\n' from end of buffer filled by fgets */
static void rmcrlf(char *s)
{
    size_t len = strlen(s);
    if (len && s[len - 1] == '\n')
        s[--len] = 0;
}

#define IN_FILE "cities.txt"

int main(int argc, char **argv)
{
    char word[WRDMAX] = "";
    char *sgl[LMAX] = {NULL};
    tst_node *root = NULL, *res = NULL;
    int rtn = 0, idx = 0, sidx = 0;
    FILE *fp = fopen(IN_FILE, "r");
    double t1, t2;
    int bench_flag = 0;

    if (argc > 1) {
        if (strcmp(argv[1], "--bench") == 0) {
            bench_flag = 1;
        }
    }

    if (!fp) { /* prompt, open, validate file for reading */
        fprintf(stderr, "error: file open failed '%s'.\n", argv[1]);
        return 1;
    }

    char *pPool = (char *) malloc(poolSize * sizeof(char));
    char *pTop = pPool;

    while ((rtn = fscanf(fp, "%s", pTop)) != EOF) {
        t1 = tvgetf();
        char *p = pTop;
        /* FIXME: insert reference to each string */
        if (!tst_ins_del(&root, &p, INS, REF)) {
            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            fclose(fp);
            return 1;
        }
        idx++;
        pTop += (strlen(pTop) + 1);
    }
    t2 = tvgetf();

    fclose(fp);
    printf("ternary_tree, loaded %d words in %.6f sec\n", idx, t2 - t1);

    for (;;) {
        char *p;
        printf(
            "\nCommands:\n"
            " a  add word to the tree\n"
            " f  find word in tree\n"
            " s  search words matching prefix\n"
            " d  delete word from the tree\n"
            " q  quit, freeing all data\n\n"
            "choice: ");
        if (bench_flag == 0) {
            fgets(word, sizeof word, stdin);
        } else {
            strcpy(word, argv[2]);
        }
        p = NULL;
        switch (*word) {
        case 'a':
            printf("enter word to add: ");
            if (bench_flag == 0) {
                if (!fgets(pTop, sizeof word, stdin)) {
                    fprintf(stderr, "error: insufficient input.\n");
                    break;
                }
                rmcrlf(pTop);
            } else {
                strcpy(pTop, argv[3]);
            }
            p = pTop;
            t1 = tvgetf();
            /* FIXME: insert reference to each string */
            res = tst_ins_del(&root, &p, INS, REF);
            t2 = tvgetf();
            if (res) {
                idx++;
                pTop += (strlen(pTop) + 1);
                printf("  %s - inserted in %.6f sec. (%d words in tree)\n",
                       (char *) res, t2 - t1, idx);
            } else
                printf("  %s - already exists in list.\n", (char *) res);
            if (bench_flag == 1) {
                free(pPool);
                return 0;
            }
            break;
        case 'f':
            printf("find word in tree: ");
            if (bench_flag == 0) {
                if (!fgets(word, sizeof word, stdin)) {
                    fprintf(stderr, "error: insufficient input.\n");
                    break;
                }
                rmcrlf(word);
            } else {
                strcpy(word, argv[3]);
            }
            t1 = tvgetf();
            res = tst_search(root, word);
            t2 = tvgetf();
            if (res)
                printf("  found %s in %.6f sec.\n", (char *) res, t2 - t1);
            else
                printf("  %s not found.\n", word);
            if (bench_flag == 1) {
                free(pPool);
                return 0;
            }
            break;
        case 's':
            printf("find words matching prefix (at least 1 char): ");
            if (bench_flag == 0) {
                if (!fgets(word, sizeof word, stdin)) {
                    fprintf(stderr, "error: insufficient input.\n");
                    break;
                }
                rmcrlf(word);
            } else {
                strcpy(word, argv[3]);
            }
            t1 = tvgetf();
            res = tst_search_prefix(root, word, sgl, &sidx, LMAX);
            t2 = tvgetf();
            if (res) {
                printf("  %s - searched prefix in %.6f sec\n\n", word, t2 - t1);
                for (int i = 0; i < sidx; i++)
                    printf("suggest[%d] : %s\n", i, sgl[i]);
            } else
                printf("  %s - not found\n", word);
            if (bench_flag == 1) {
                free(pPool);
                return 0;
            }
            break;
        case 'd':
            printf("enter word to del: ");
            if (bench_flag == 0) {
                if (!fgets(word, sizeof word, stdin)) {
                    fprintf(stderr, "error: insufficient input.\n");
                    break;
                }
                rmcrlf(word);
            } else {
                strcpy(word, argv[3]);
            }
            p = word;
            printf("  deleting %s\n", word);
            t1 = tvgetf();
            /* FIXME: remove reference to each string */
            res = tst_ins_del(&root, &p, DEL, REF);
            t2 = tvgetf();
            if (res)
                printf("  delete failed.\n");
            else {
                printf("  deleted %s in %.6f sec\n", word, t2 - t1);
                idx--;
            }
            if (bench_flag == 1) {
                free(pPool);
                return 0;
            }
            break;
        case 'q':
            tst_free(root);
            free(pPool);
            return 0;
            break;
        default:
            fprintf(stderr, "error: invalid selection.\n");
            break;
        }
    }

    return 0;
}
