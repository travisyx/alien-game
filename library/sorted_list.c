 #include "sorted_list.h"

// Initializes a sorted list
slist_t *sl_init(int size, free_func_t freer){
  slist_t *ans = malloc(sizeof(slist_t));
  // Not going to compile, add a freer
  ans->list = list_init(size, freer);
  return ans;
}

// Frees a sorted list
void sl_free(slist_t *sl){
  list_free(sl->list);
  free(sl);
}

// Enqueues the item, changing priority if necessary
void sl_enqueue(slist_t *slist, node_t *item){
  int counter = list_size(slist->list)-1;
  list_add(slist->list, item);
  double curr_pri = item->priority;
  while(counter >= 0){
    if(curr_pri < ((node_t *)list_get(slist->list, counter))->priority)
      break;
    else{
      list_swap(slist->list, counter, counter + 1);
      counter--;
    }
  }
}

// Returns the size of the sorted list
size_t sl_size(slist_t *slist){
  return list_size(slist->list);
}

// Peeks the last element without removing
void *sl_peek(slist_t *slist){
  // return (void *) list_get(slist->list, 0);
  return (void *) list_get(slist->list, sl_size(slist) - 1);
}

// Removes the 0th index element
void *sl_dequeue(slist_t *slist){
  return list_remove(slist->list, sl_size(slist)-1);
}

// Compares if two nodes are the same
bool node_compare(node_t *first, node_t *second){
  vector_t one = body_get_centroid(first->node->body);
  vector_t two = body_get_centroid(second->node->body);
  if(one.x == two.x && one.y == two.y)
    return true;
  return false;
}

// Changes the priority of an element of the sorted list
void sl_change_priority(slist_t *slist, node_t *item, double priority){
  size_t ind = 0;
  item->priority = priority;
  for(size_t i = 0; i < list_size(slist->list); i++){
    node_t *temp = (node_t *) list_get(slist->list, i);
    if(node_compare(temp, item)){
      ind = i;
      break;
    }
  }
  list_remove(slist->list, ind);
  sl_enqueue(slist, item);
}

// Prints the centroid of each node in the list. MUST have a list of nodes
void sl_print(slist_t *slist){
  list_t *vals = slist->list;
  printf("new path\n");
  for(size_t i = 0; i < list_size(vals); i++){
    node_t *val = (node_t *)list_get(vals, i);
    vector_t centr = body_get_centroid(val->node->body);
    printf("%f %f, %f \n", centr.x, centr.y, val->priority);
  }
  printf("\n");
}
