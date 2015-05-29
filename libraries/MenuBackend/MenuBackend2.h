/*
||
|| @file MenuBackend.h
|| @version 1.1-lite
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
|| @note some minor changes Adrian Brzezinski adrb@wp.pl
||
|| @description
|| | Provide an easy way of making menus
|| #
||
|| @license
|| | This library is free software; you can redistribute it and/or
|| | modify it under the terms of the GNU Lesser General Public
|| | License as published by the Free Software Foundation; version
|| | 2.1 of the License.
|| |
|| | This library is distributed in the hope that it will be useful,
|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|| | Lesser General Public License for more details.
|| |
|| | You should have received a copy of the GNU Lesser General Public
|| | License along with this library; if not, write to the Free Software
|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|| #
||
*/

#ifndef MenuBackend_h
#define MenuBackend_h

class MenuItem {
public:

     MenuItem(const char* itemName, MenuItem *backpoint=0 ) : name(itemName), back(backpoint) {
           before = 0;
           right = 0;
           after = 0;
           left = 0;
     }

     const char* getName() const { return name; }
     MenuItem* getBack() { return back; }
     MenuItem* getBefore() { return before; }
     MenuItem* getRight() { return right; }
     MenuItem* getAfter() { return after; }
     MenuItem* getLeft() { return left; }

     MenuItem* moveUp() { return before; }
     MenuItem* moveDown() { return after; }
     MenuItem* moveLeft() { return left; }
     MenuItem* moveRight() { return right; }

     //default vertical menu
     MenuItem &add(MenuItem &mi) { return addAfter(mi); }

     MenuItem &addBefore(MenuItem &mi) {
           mi.after = this;
           before = &mi;
           if ( !mi.back ) mi.back = back;
           return mi;
     }
     MenuItem &addRight(MenuItem &mi) {
           mi.left = this;
           right = &mi;
           if ( !mi.back ) mi.back = back;
           return mi;
     }
     MenuItem &addAfter(MenuItem &mi) {
           mi.before = this;
           after = &mi;
           if ( !mi.back ) mi.back = back;
           return mi;
     }
     MenuItem &addLeft(MenuItem &mi) {
           mi.right = this;
           left = &mi;
           if ( !mi.back ) mi.back = back;
           return mi;
     }
protected:

     const char* name;

     MenuItem *before;
     MenuItem *right;
     MenuItem *after;
     MenuItem *left;
     MenuItem *back;
};

//no dependant inclusion of string or cstring
bool menuTestStrings(const char *a, const char *b) {
     while (*a) { if (*a != *b) { return false; } b++; a++; }
     return true;
}
bool operator==(MenuItem &lhs, char* test) {
     return menuTestStrings(lhs.getName(),test);
}
bool operator==(const MenuItem &lhs, char* test) {
     return menuTestStrings(lhs.getName(),test);
}
bool operator==(MenuItem &lhs, MenuItem &rhs) {
     return menuTestStrings(lhs.getName(),rhs.getName());
}
bool operator==(const MenuItem &lhs, MenuItem &rhs) {
     return menuTestStrings(lhs.getName(),rhs.getName());
}

struct MenuChangeEvent {
     const MenuItem &from;
     const MenuItem &to;
};

struct MenuUseEvent {
     const MenuItem &item;
};

typedef void (*cb_change)(MenuChangeEvent);
typedef void (*cb_use)(MenuUseEvent);

class MenuBackend {
public:

     MenuBackend(cb_use menuUse, cb_change menuChange = 0) : root("MenuRoot") {
           current = &root;
           cb_menuChange = menuChange;
           cb_menuUse = menuUse;
     }

     MenuItem &getRoot() {
           return root;
     }
     MenuItem &getCurrent() {
           return *current;
     }

     void moveBack() {
           setCurrent(current->getBack());
     }

     void moveUp() {
           setCurrent(current->moveUp());
     }

     void moveDown() {
           setCurrent(current->moveDown());
     }

     void moveLeft() {
           setCurrent(current->moveLeft());
     }

     void moveRight() {
           setCurrent(current->moveRight());
     }

     void use() {
           if (cb_menuUse) {
                 MenuUseEvent mue = { *current };
                 cb_menuUse(mue);
           }
     }

private:
     void setCurrent( MenuItem *next ) {
           if (!next) return;

           if (cb_menuChange) {
                 MenuChangeEvent mce = { *current, *next };
                 cb_menuChange(mce);
           }

           current = next;
     }

     MenuItem root;
     MenuItem *current;

     cb_change cb_menuChange;
     cb_use cb_menuUse;
};

#endif