/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.9  2005/08/30 21:08:51  arman
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */




static bool nocase_compare (char c1, char c2)
{
	return toupper(c1) == toupper (c2);
}

bool rfNocaseEqual (const std::string& s1, const std::string& s2)
{
	return equal (s1.begin (), s1.end (), s2.begin (), nocase_compare);
}

