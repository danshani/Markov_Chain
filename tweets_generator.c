#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "markov_chain.h"

#define MAX_TWEET_LENGTH 20
#define MAX_LINE_LENGTH 1000
#define DELIMITERS " \n\r\t"

int main(int argc, char *argv[]) {
    // Check arguments
    if (argc != 4 && argc != 5) {
        fprintf(stdout, "Usage: %s <seed> <tweets_count> <input_file> [words_to_read]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    // Parse arguments
    unsigned int seed = (unsigned int) strtol(argv[1], NULL, 10);
    int tweets_count = (int) strtol(argv[2], NULL, 10);
    int words_to_read = -1;

    if (argc == 5) {
        words_to_read = (int) strtol(argv[4], NULL, 10);
    }

    srand(seed); // Set random seed

    // Open input file
    FILE *fp = fopen(argv[3], "r");
    if (fp == NULL) {
        fprintf(stdout, "Error: Cannot open file %s\n", argv[3]);
        return EXIT_FAILURE;
    }

    // Initialize Markov Chain
    MarkovChain *chain = malloc(sizeof(MarkovChain));
    if (chain == NULL) {
        fclose(fp);
        return EXIT_FAILURE;
    }

    // Initialize database
    chain->database = malloc(sizeof(LinkedList));
    if (chain->database == NULL) {
        free(chain);
        fclose(fp);
        return EXIT_FAILURE;
    }
    chain->database->first = NULL;
    chain->database->last = NULL;
    chain->database->size = 0;

    // Process input file
    char line[MAX_LINE_LENGTH];
    int word_counter = 0;
    Node *prev_node = NULL;

    while (fgets(line, sizeof(line), fp) != NULL && (words_to_read == -1 || word_counter < words_to_read)) {
        char *word = strtok(line, DELIMITERS);

        while (word != NULL && (words_to_read == -1 || word_counter < words_to_read)) {
            Node *current_node = add_to_database(chain, word);
            if (current_node == NULL) {
                free_database(&chain);
                fclose(fp);
                return EXIT_FAILURE;
            }

            if (prev_node != NULL) {
                if (add_node_to_frequency_list(prev_node->data,
                                               current_node->data) != 0) {
                    free_database(&chain);
                    fclose(fp);
                    return EXIT_FAILURE;
                }
            }

            word_counter++;
            prev_node = current_node;
            word = strtok(NULL, DELIMITERS);
        }
    }

    // Generate tweets
    for (int i = 1; i <= tweets_count; i++) {
        fprintf(stdout, "Tweet %d: ", i);
        MarkovNode *first = get_first_random_node(chain);
        generate_tweet(first, MAX_TWEET_LENGTH);
    }

    // Cleanup
    free_database(&chain);
    fclose(fp);
    return EXIT_SUCCESS;
}