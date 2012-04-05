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

#ifndef __WINDOWSLIST_H__
#define __WINDOWSLIST_H__

#include "Window.h"

class WindowsList
{
public:
   WindowsList();
   ~WindowsList();

   class Iterator;

   class Node
   {
      friend class Iterator;
      friend class WindowsList;

   public:
      Node(HWND hWnd): m_window(hWnd) { return; }

      operator Window& ()  { return m_window; }
      operator Window* ()  { return &m_window; }
      Node * next() const  { return m_next; }
      Node * prev() const  { return m_prev; }

   protected:
      Node * m_prev;
      Node * m_next;

      Window m_window;
   };

   class Iterator
   {
   public:
      Iterator(): m_list(NULL), m_node(NULL) { return; }
      Iterator(WindowsList * list, Node* node): m_list(list), m_node(node) { return; }

      operator Window*()         { return &m_node->m_window; }
      Iterator& operator ++(int) { Iterator * tmp = this; m_node = m_node->m_next; return *tmp; }
      Iterator& operator ++()    { m_node = m_node->m_next; return *this; }
      Iterator& operator --(int) { Iterator * tmp = this; m_node = m_node->m_prev; return *tmp; }
      Iterator& operator --()    { m_node = m_node->m_prev; return *this; }
      operator bool()            { return m_node != (Node*)&m_list->m_extrem; }
      void Erase();
      void InsertAfter(Node* node);
      void InsertBefore(Node* node);
      void MoveToEnd();
      void MoveToBegin();

   protected:
      WindowsList * m_list;
      Node * m_node;
   };

   void push_back(Node * node);
   void push_front(Node * node);

   Node * pop_back();
   Node * pop_front();

   Node * back() const     { return m_extrem.tail; }
   Node * front() const    { return m_extrem.head; }

   bool empty() const      { return m_extrem.head == (Node*)&m_extrem; }
   void clear();

   Iterator begin()        { return Iterator(this, (Node*)m_extrem.head); }
   Iterator end()          { return Iterator(this, (Node*)&m_extrem); }

   Iterator first()        { return Iterator(this, (Node*)m_extrem.head); }
   Iterator last()         { return Iterator(this, (Node*)m_extrem.tail); }

protected:
   class ExtremNode {
   public:
      Node * tail;
      Node * head;
   } m_extrem;
};

#endif /*__WINDOWSLIST_H__*/
