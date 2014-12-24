#include "clips.h"

#ifndef PHP_CLIPS_ENV_LIST
#define PHP_CLIPS_ENV_LIST 1

#include "../config.h"
#include "clips/clips.h"

struct env_list_node {
	char* s_name;
	void* pv_env;
	struct env_list_node* next;
};

typedef struct env_list_node* EnvironmentListNode;

extern EnvironmentListNode env_list;

EnvironmentListNode current_env_list_node();
EnvironmentListNode create_env_list_node(const char* s_name);
EnvironmentListNode last_env_list_node();
EnvironmentListNode get_env_list_node(const char* s_name);
void* create_env(const char* s_name);
void* get_env(const char* s_name);
bool switch_env(const char* s_name);
void destroy_envlist();

#endif
