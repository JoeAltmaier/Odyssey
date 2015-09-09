#include "SamuraiServer.h"

// add a command string to our current command strings queue
void node::enqueue()
{
	node * tmp = head;
	node * tmptail = tail;

	if (! isEmpty())
	{
		tmptail->next = this;
		tail = this;
	}
	else
	{
		tmp = this;
		tmptail = this;
		head = tmp;
		tail = tmptail;
	}
	tail->next = NULL;
}

int node::dequeue()
{
	node * tmp;
	
	if (isEmpty())
		return(-32);
	else
	{
		tmp = head;
		if (tmp->next == NULL)
		{
			head = NULL;
			delete(tmp);
			return(0);
		}
		head = tmp->next;
		head->prev = NULL;
		delete (tmp);
	}
	return (0);
}

bool node::isEmpty()	
{
	if (head == NULL)
		return (true);
	else
		return (false);
} 

