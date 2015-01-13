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
    shared_ptr<Cached> cache(new Cached());

    {
        shared_ptr<Cached::buffer> p3 = cache->get(1000*1200);
    }

    shared_ptr<Cached::buffer> p1 = cache->get(1000*1200);
}

