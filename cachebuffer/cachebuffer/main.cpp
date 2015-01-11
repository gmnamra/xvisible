//
//  main.cpp
//  cachebuffer
//
//  Created by Arman Garakani on 1/10/15.
//  Copyright (c) 2015 Arman Garakani. All rights reserved.
//

#include <iostream>
#include "membufCache.hpp"

int main(int argc, const char * argv[])
{
    shared_ptr<Cache> cache(new Cache());
    shared_ptr<Book> p0 = cache->get("book1");
    

        {
            shared_ptr<Book> p2 = cache->get("book1");
            {
                shared_ptr<Book> p3 = cache->get("book1");
            }

            
            shared_ptr<Book> p4 = cache->get("book2");
        }//1
        //4
        shared_ptr<Book> p1 = cache->get("book5");
        shared_ptr<Book> p2 = cache->get("book6");
}
