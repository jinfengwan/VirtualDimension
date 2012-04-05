/* 
 * Virtual Dimension -  a free, fast, and feature-full virtual desktop manager 
 * for the Microsoft Windows platform.
 * Copyright (C) 2003-2008 Francois Ferrand
 *
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later 
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with 
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "StdAfx.h"
#include "WindowsList.h"

WindowsList::WindowsList(void)
{
   m_extrem.head = (Node*)&m_extrem;
   m_extrem.tail = (Node*)&m_extrem;
}

WindowsList::~WindowsList(void)
{
   clear();
}

void WindowsList::push_back(WindowsList::Node * node)
{
   Iterator it = end();
   it.InsertBefore(node);
}

void WindowsList::push_front(WindowsList::Node * node)
{
   Iterator it = begin();
   it.InsertBefore(node);
}

WindowsList::Node * WindowsList::pop_back()
{
   Node * node = m_extrem.tail;
   
   m_extrem.tail = node->m_prev;
   m_extrem.tail->m_next = (Node*)&m_extrem;
   
   return node;
}

WindowsList::Node * WindowsList::pop_front()
{
   Node * node = m_extrem.head;

   m_extrem.head = node->m_next;
   m_extrem.head->m_prev = (Node*)&m_extrem;
   
   return node;
}

void WindowsList::clear()
{
   Node * node = m_extrem.head;
   while(node != (Node*)&m_extrem)
   {
      Node * next = node->m_next;
      delete node;
      node = next;
   }
   m_extrem.head = (Node*)&m_extrem;
   m_extrem.tail = (Node*)&m_extrem;
}

void WindowsList::Iterator::Erase()
{
   if (!(bool)*this)
      return;

   m_node->m_prev->m_next = m_node->m_next;
   m_node->m_next->m_prev = m_node->m_prev;

   delete m_node;
   m_node = NULL;
}

void WindowsList::Iterator::InsertAfter(Node* node)
{
   node->m_prev = m_node;
   node->m_next = m_node->m_next;
   m_node->m_next = node;
   node->m_next->m_prev = node;
}

void WindowsList::Iterator::InsertBefore(Node* node)
{
   node->m_next = m_node;
   node->m_prev = m_node->m_prev;
   m_node->m_prev = node;
   node->m_prev->m_next = node;
}

void WindowsList::Iterator::MoveToEnd()
{
   m_node->m_prev->m_next = m_node->m_next;
   m_node->m_next->m_prev = m_node->m_prev;

   m_node->m_prev = m_list->m_extrem.tail;
   m_list->m_extrem.tail->m_next = m_node;
   m_node->m_next = (Node*)&m_list->m_extrem;
   m_list->m_extrem.tail = m_node;
}

void WindowsList::Iterator::MoveToBegin()
{
   m_node->m_prev->m_next = m_node->m_next;
   m_node->m_next->m_prev = m_node->m_prev;

   m_node->m_next = m_list->m_extrem.head;
   m_list->m_extrem.head->m_prev = m_node;
   m_node->m_prev = (Node*)&m_list->m_extrem;
   m_list->m_extrem.head = m_node;
}
