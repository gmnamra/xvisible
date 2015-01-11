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
    shared_ptr<ImageBuffer> p0 = cache->get(800*600);
        {
            shared_ptr<ImageBuffer> p2 = cache->get(800*600);
            {
                shared_ptr<ImageBuffer> p3 = cache->get(1000*1200);
            }

            
            shared_ptr<ImageBuffer> p4 = cache->get(480000);
        }//1
        //4
        shared_ptr<ImageBuffer> p1 = cache->get(1000*1200);
        shared_ptr<ImageBuffer> p2 = cache->get(800*600);
}
