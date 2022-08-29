/*************************************
* Lab 1 Exercise 2
* Name: Vikrant Prakash
* Student No: A0227630L
* Lab Group: 3
*************************************/

#include "node.h"
#include <stdlib.h>
#include <stdio.h>


// Add in your implementation below to the respective functions
// Feel free to add any headers you deem fit 


// Inserts a new node with data value at index (counting from head
// starting at 0).
// Note: index is guaranteed to be valid.
void insert_node_at(list *lst, int index, int data) {
    node *newNode;
    newNode = (node *) malloc(sizeof(node));
    (*newNode).data = data;
    if(lst->head == NULL) {
        lst->head = newNode;
    } else if(index == 0) {
        node *temp = lst->head;
        lst->head = newNode;
        (*newNode).next = temp;
    } else {
        int i;
        node *curr = lst->head;
        int last = 0;
        for(i = 0; i < index - 1; i++) {
            if((*curr).next == NULL) {
                last = 1;
                break;
            }
            curr = (*curr).next;
        }
        if(last == 0) {
            node *n = (*curr).next;
            (*curr).next = newNode;
            (*newNode).next = n;
        } else {
            (*curr).next == newNode;
        }
    }
    return;
}

// Deletes node at index (counting from head starting from 0).
// Note: index is guarenteed to be valid.
void delete_node_at(list *lst, int index) {
    printf("hey ");
    if(index == 0) {
        printf("0 ");
        if((*(lst->head)).next == NULL) {
            lst->head = NULL;
        } else {
            node* p = (*(lst->head)).next;
            lst->head = p;
            free((*(lst->head)).next);
        }
    } else {
        printf("not0 ");
        node* curr = lst->head;
        int i;
        for(i = 1; i < index; i++) {
            curr = (*curr).next;
        }
        if((*((*curr).next)).next == NULL) {
            (*curr).next = NULL;
        } else {
            node* del = (*((*curr).next)).next;
            (*curr).next = del;
        }
    }
    return;
}

// Search list by the given element.
// If element not present, return -1 else return the index. If lst is empty return -2.
//Printing of the index is already handled in ex2.c
int search_list(list *lst, int element) {




}

// Reverses the list with the last node becoming the first node.
void reverse_list(list *lst) {



}

// Resets list to an empty state (no nodes) and frees
// any allocated memory in the process
void reset_list(list *lst) {


}
