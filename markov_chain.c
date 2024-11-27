#include "markov_chain.h"

int get_random_number(int max_number){
    return rand() % max_number;
}

bool ends_with_period(const char *str) {
    // check if the last character is a period
    return str[strlen(str) - 1] == '.';

}

Node* get_node_from_database(MarkovChain *markov_chain, char *data_ptr){
    if (markov_chain == NULL || markov_chain->database == NULL){
        return NULL;
    }
    Node *current_node = markov_chain->database->first;
    while (current_node != NULL){
        if (strcmp(current_node->data->data, data_ptr) == 0){
            return current_node;
        }
        current_node = current_node->next;
    }
    return NULL;
}

Node* add_to_database(MarkovChain *markov_chain, char *data_ptr) {
    if (markov_chain == NULL || markov_chain->database == NULL) {
        return NULL;
    }

    // Check if node already exists
    Node *existing_node = get_node_from_database(markov_chain, data_ptr);
    if (existing_node != NULL) {
        return existing_node;
    }

    // Create new MarkovNode
    MarkovNode *new_markov_node = malloc(sizeof(MarkovNode));
    if (new_markov_node == NULL) {
        printf(ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }

    // Copy the data
    new_markov_node->data = strdup(data_ptr);
    if (new_markov_node->data == NULL) {
        free(new_markov_node);
        return NULL;
    }

    // Initialize frequency list
    new_markov_node->frequency_list = NULL;
    new_markov_node->frequency_list_size = 0;

    // Add to database first
    if (add(markov_chain->database, new_markov_node) != 0) {
        free(new_markov_node->data);
        free(new_markov_node);
        return NULL;
    }

    // Get the node that was just added
    return get_node_from_database(markov_chain, data_ptr);
}

void free_frequency_list(MarkovNode *node) {
    if (node != NULL && node->frequency_list != NULL) {
        free(node->frequency_list);
        node->frequency_list = NULL;
        node->frequency_list_size = 0;
    }
}

void free_database(MarkovChain **ptr_chain) {
    if (ptr_chain == NULL || *ptr_chain == NULL) {
        return;
    }
    MarkovChain *markov_chain = *ptr_chain;
    if (markov_chain->database != NULL) {
        Node *current_node = markov_chain->database->first;
        while (current_node != NULL) {
            MarkovNode *markov_node = current_node->data;
            free_frequency_list(markov_node);
            free(markov_node->data);
            free(markov_node);
            Node *next_node = current_node->next;
            free(current_node);
            current_node = next_node;
        }
        free(markov_chain->database);
    }
    free(markov_chain);
    *ptr_chain = NULL;
}

int add_node_to_frequency_list(MarkovNode *first_node, MarkovNode *second_node) {
    if (first_node == NULL || second_node == NULL) {
        return 1;
    }

    // Check if the second_node is already in the frequency list
    for (int i = 0; i < first_node->frequency_list_size; i++) {
        if (first_node->frequency_list[i].markov_node == second_node) {
            first_node->frequency_list[i].frequency++;
            return 0;
        }
    }

    // If not found, add it to the frequency list
    MarkovNodeFrequency *new_list = realloc(first_node->frequency_list,
                                            (first_node->frequency_list_size + 1) * sizeof(MarkovNodeFrequency));
    if (new_list == NULL) {
        return 1;
    }

    first_node->frequency_list = new_list;
    first_node->frequency_list[first_node->frequency_list_size].markov_node = second_node;
    first_node->frequency_list[first_node->frequency_list_size].frequency = 1;
    first_node->frequency_list_size++;

    return 0;
}

MarkovNode* get_first_random_node(MarkovChain *markov_chain) {
    // Check if the chain is empty or NULL or if the database is empty
    if (markov_chain == NULL || markov_chain->database == NULL || markov_chain->database->size == 0) {
        return NULL;
    }

    Node *current_node;
    // Get random node until it's not the one ending with period
    do {
        int random_index = get_random_number(markov_chain->database->size);
        current_node = markov_chain->database->first;
        for (int i = 0; i < random_index; i++) {
            current_node = current_node->next;
        }
    }
    // Check if the node ends with period
    while (ends_with_period(current_node->data->data));

    return current_node->data;
}

MarkovNode* get_next_random_node(MarkovNode *cur_markov_node){
    if (cur_markov_node == NULL || cur_markov_node->frequency_list == NULL ||
        cur_markov_node->frequency_list_size == 0){
        return NULL;
    }

    int total_frequency = 0;
    for (int i = 0; i < cur_markov_node->frequency_list_size; i++){
        total_frequency += cur_markov_node->frequency_list[i].frequency;
    }
    int random_frequency = get_random_number(total_frequency);
    int current_frequency = 0;

    for (int i = 0; i < cur_markov_node->frequency_list_size; i++){
        current_frequency += cur_markov_node->frequency_list[i].frequency;
        if (current_frequency > random_frequency){
            return cur_markov_node->frequency_list[i].markov_node;
        }
    }
    return NULL;
}

void generate_tweet(MarkovNode *first_node, int max_length) {
    if (first_node == NULL) {
        return;
    }

    int words_count = 0;
    MarkovNode *current = first_node;

    // Print first word
    printf("%s", current->data);
    words_count++;

    while (words_count < max_length) {
        // Get next random node
        current = get_next_random_node(current);
        if (current == NULL) {
            break;
        }

        printf(" %s", current->data);
        words_count++;

        // Check for period at end of word
        if (current->data[strlen(current->data) - 1] == '.') {
            break;
        }
    }
    printf("\n");
}