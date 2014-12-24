#include "environment_list.h"

EnvironmentListNode env_list = NULL;

EnvironmentListNode last_env_list_node() {
	if(env_list) {
		EnvironmentListNode parent = env_list;
		while(parent->next) {
			parent = parent->next;
		}
		return parent;
	}
	return NULL;
}

EnvironmentListNode current_env_list_node() {
	if(p_clips_env) {
		EnvironmentListNode node = env_list;
		while(node) {
			if(node->pv_env == p_clips_env)
				return node;
			node = node->next;
		}
	}
	return NULL;
}

EnvironmentListNode create_env_list_node(const char* s_name) {
	EnvironmentListNode node = (EnvironmentListNode) malloc(sizeof(struct env_list_node));
	if(node) {
		node->s_name = (char *) malloc(strlen(s_name) + 1);
		strcpy(node->s_name, (char *) s_name);
		node->pv_env = CreateEnvironment(); // Let's create the environment
		node->next = NULL;
		return node;
	}
	return NULL;
}

EnvironmentListNode get_env_list_node(const char* s_name) {
	EnvironmentListNode node = env_list;
	while(node) {
		if(strcmp(node->s_name, s_name) == 0) {
			return node;
		}
		node = node->next;
	}
	return NULL;
}

void* get_env(const char* s_name) {
	EnvironmentListNode node = get_env_list_node(s_name);
	if(node && node->pv_env)
		return node->pv_env;
	return NULL;
}

void* create_env(const char* s_name) {
	void * env = get_env(s_name); // If we can find the env, need not to create it again
	if(env)
		return env;

	if(s_name) {
		// Get the parent first
		EnvironmentListNode parent = last_env_list_node();

		if(!parent) { // We didn't have env_list for now, let's create it
			env_list = create_env_list_node(s_name);
			return env_list->pv_env;
		}
		else {
			EnvironmentListNode node = create_env_list_node(s_name);
			parent->next = node;
			return node->pv_env;
		}
	}
	return NULL;
}

bool switch_env(const char* s_name) {
	void* env = get_env(s_name);
	if(env) {
		p_clips_env = get_env(s_name);
		return true;
	}
	return false;	
}

EnvironmentListNode parent_env_list_node(EnvironmentListNode node) {
	if(env_list) {
		EnvironmentListNode parent = env_list;
		while(parent) {
			if(parent->next == node)
				return parent;
			parent = parent->next;
		}
	}
	return NULL;
}

void destroy_envlist() {
	if(env_list) {
		EnvironmentListNode node = last_env_list_node();
		while(true) {
			if(node->pv_env) { // Destroy the env
				DestroyEnvironment(node->pv_env);
				free((void*)node->s_name);
			}

			// Update the parent
			EnvironmentListNode parent = parent_env_list_node(node);

			if(parent) { // If we can find the parent
				parent->next = NULL; // Update parent to be the last one
				free(node); // Free the node
				node = last_env_list_node();
				continue;
			}

			if(node == env_list) { // If this is already the root
				env_list = NULL;
				free(node); // Free the node
				break;
			}
		}
	}
}
