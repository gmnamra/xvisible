/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/10/17 15:49:18  arman
 *proto of preferences support
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcPREFERENCES_H
#define __rcPREFERENCES_H

class Preferences {
public:
    // constructor current app / current user
    // compiler generated ctor copy ctor dtor ok
    Preferences();


    // preference file information
    const QString& file();
    const QString& format();
    const QString& version();

    // boolean data storage
    bool get(const CFStringRef key, bool def = false);
    void set(const CFStringRef key, bool value);
    // integer data storage
    long get(const CFStringRef key, long def = 0);
    void set(const CFStringRef key, long value);
    // double data storage
    double get(const CFStringRef key, double def = 0.0);
    void set(const CFStringRef key, double value);
    // string data storage
    QString get(const CFStringRef key);
    void set(const CFStringRef key, const CFStringRef value);

    // synch preferences
  void sync ();

  
};


#endif /* __rcPREFERENCES_H */
