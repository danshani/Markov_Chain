#include "markov_chain.h"

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number){
    return rand() % max_number;
}

/**
 * Helper -> bool function to check if a string ends with a period
 * @param str the string to check
 * @return true if the string ends with a period, false otherwise
 */
bool ends_with_period(const char *str) {
    // check if the last character is a period
    return str[strlen(str) - 1] == '.';

}

/**
* Check if data_ptr is in database. If so, return the Node wrapping it in
 * the markov_chain, otherwise return NULL.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the data to look for
 * @return Pointer to the Node wrapping given data, NULL if state not in
 * database.
 */
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

/**
* If data_ptr in markov_chain, return it's node. Otherwise, create new
 * node, add to end of markov_chain's database and return it.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the data to look for
 * @return Node wrapping given data_ptr in given chain's database,
 * returns NULL in case of memory allocation failure.
 */
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

    // Create new Node to wrap MarkovNode
    Node *new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        free(new_markov_node->data);
        free(new_markov_node);
        return NULL;
    }

    // Set up the new node
    new_node->data = new_markov_node;
    new_node->next = NULL;

    // Add to database
    if (add(markov_chain->database, new_markov_node) != 0) {
        free(new_markov_node->data);
        free(new_markov_node);
        free(new_node);
        return NULL;
    }

    return new_node;
}

/**
 * Free markov_chain and all of it's content from memory
 * @param markov_chain markov_chain to free
 */
void free_database(MarkovChain **ptr_chain){
    if (ptr_chain == NULL || *ptr_chain == NULL){
        return;
    }
    MarkovChain *markov_chain = *ptr_chain;
    if (markov_chain->database != NULL){
        Node *current_node = markov_chain->database->first;
        while (current_node != NULL){
            free(current_node->data->data);
            free(current_node->data);
            Node *next_node = current_node->next;
            free(current_node);
            current_node = next_node;
        }
        free(markov_chain->database);
    }
    free(markov_chain);
    *ptr_chain = NULL;
}

/**
 * Add the second markov_node to the frequency list of the first markov_node.
 * If already in list, update it's occurrence frequency value.
 * @param first_node
 * @param second_node
 * @return success/failure: 0 if the process was successful, 1 if in
 * case of allocation error.
 */
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

/**
 * Get one random MarkovNode from the given markov_chain's database.
 * @param markov_chain
 * @return the random MarkovNode
 */
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

/**
 * Choose randomly the next MarkovNode, depend on it's occurrence frequency.
 * @param cur_markov_node current MarkovNode
 * @return the next random MarkovNode
 */
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

/**
 * Receive markov_chain, generate and print random sentence out of it. The
 * sentence must have at least 2 words in it.
 * @param first_node markov_node to start with
 * @param  max_length maximum length of chain to generate
 */
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
