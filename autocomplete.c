#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "autocomplete.h"

int comp_terms_sortlex(const void *a, const void *b){
    // Comparator helper function for terms_sortlex
    return strcmp((*(term*)a).term, (*(term*)b).term);
}

int comp_terms_sortweight(const void *a, const void *b){
    // Sorts terms by increasing weight
    return ((*(term*)b).weight - (*(term*)a).weight);
}


void read_in_terms(term **terms, int *pnterms, char *filename){
    FILE *fp = fopen(filename, "r");
    if (fp == NULL){
        printf("%s\n", "ERROR: File read failure.");
        exit(1);
    }
    char line[225];
    fgets(line, sizeof(line), fp); // Takes the "no of lines" number in first line
    int nterms = atoi(line);

    // Modifies stuff outside to reflect file properties
    *terms = (term *)malloc(sizeof(term)*nterms);
    *pnterms = nterms;

    for (int n = 0; n < nterms; n++){
        fgets(line, sizeof(line), fp);

        // Splits weight and term by delimiters
        int place = 0;
        int wtend = 0;
        int termstart = 0;
        while(place < 225){
            if(! isspace(line[place])){
                wtend = place;
            } else if(wtend != 0 && isspace(line[place])){
                termstart = place + 1;
                break;
            }
            place++;
        }

        // Writes to term structs
        strncpy((*terms)[n].term, line + termstart, 200);
        char *wt = (char *)malloc(25);
        strncpy(wt, line, wtend+1);
        wt[wtend+1] = '\0';
        (*terms)[n].weight = (float)atoll(wt);
        free(wt);

        // Eliminate newlines
        int termlen = strlen((*terms)[n].term);
        if ((*terms)[n].term[termlen - 1] == '\n'){
            (*terms)[n].term[termlen - 1] = '\0';
        }
    }

    qsort(*terms, nterms, sizeof(term), comp_terms_sortlex);
    fclose(fp);
}


int lowest_match(term *terms, int nterms, char *substr){
    // Returns the index in terms of the first term in lexicographic ordering that matches the string substr.
    int left = 0;
    int right = nterms - 1;
    int mid;
    while(left <= right){
        mid = (int)floor((left+right)/2);
        if(strcmp((terms+mid)->term, substr) < 0){ //If terms[mid] is to left of substr
            left = mid + 1; //move lower bound up
        } else{
            right = mid - 1; //move upper bound down
        }
    }
    return left;
}


int highest_match(term *terms, int nterms, char *substr){
    // Returns the index in terms of the last term in lexicographic order that matches the string substr.
    int left = lowest_match(terms, nterms, substr);
    int right = nterms - 1;
    int mid;
    int match = -1;
    while(left <= right){
        mid = (int)floor((left+right)/2);
        if(strncmp((terms+mid)->term, substr, strlen(substr)) <= 0){ //If terms[mid] is to left of substr (still starts with similar things)
            left = mid + 1; //Move lower bound up
            match = mid;
        } else{
            right = mid - 1; //Move upper bound down
        }
    }
    return match;
}

void autocomplete(term **answer, int *n_answer, term *terms, int nterms, char *substr){
    // Takes terms (assume it is sorted lexicographically in increasing order), the number of terms nterms, and the query string substr, and places the answers in answer, with *n_answer being the number of answers.
    // Answers should be sorted by weight in non-increasing order.
    int lowest = lowest_match(terms, nterms, substr);
    int highest = highest_match(terms, nterms, substr); //possibly need to +1 for case when lowest = highest?
    int n_ans = highest - lowest + 1;
    *n_answer = n_ans;

    if(n_ans <= 0){
        return;
    } else{
        // Creates and modifies answer array of terms (THIS WORKS FINE. Only outputs 0 because highest = lowest)
        *answer = (term *)malloc(sizeof(term)*n_ans);
        for(int a = lowest; a <= highest; a++){
            (*answer + a - lowest) -> weight = terms[a].weight;
            strcpy((*answer + a - lowest) -> term, terms[a].term);
        }
    }
    qsort(*answer, n_ans, sizeof(term), comp_terms_sortweight);

}
