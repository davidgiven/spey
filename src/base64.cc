/*
 * Copyright (c) 1995 - 1999 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* This is a slightly tweaked copy of the base64 encode/decode routines taken
 * from the krb4-1.0.4a package, which can be found here:
 * 
 * ftp://ftp.pdc.kth.se/pub/krb/src/krb4-1.0.4a.tar.gz
 * 
 * All of my changes to these code are in the public domain.
 * 
 * David Given dg@cowlark.com 2008-01-05
 */

#include <stdlib.h>
#include <string.h>
#include "common.h"

static char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int pos(char c)
{
  char *p;
  for(p = base64; *p; p++)
    if(*p == c)
      return p - base64;
  return -1;
}

string base64_encode(const void* data, int size)
{
	char* str;
	int len = base64_encode(data, size, &str);
	
	if (len == -1)
		return "";
	return string(str, len);
}
	
int base64_encode(const void *data, int size, char **str)
{
  char *s, *p;
  int i;
  int c;
  const unsigned char *q;

  p = s = (char*)malloc(size*4/3+4);
  if (p == NULL)
      return -1;
  q = (const unsigned char*)data;
  i=0;
  for(i = 0; i < size;){
    c=q[i++];
    c*=256;
    if(i < size)
      c+=q[i];
    i++;
    c*=256;
    if(i < size)
      c+=q[i];
    i++;
    p[0]=base64[(c&0x00fc0000) >> 18];
    p[1]=base64[(c&0x0003f000) >> 12];
    p[2]=base64[(c&0x00000fc0) >> 6];
    p[3]=base64[(c&0x0000003f) >> 0];
    if(i > size)
      p[3]='=';
    if(i > size+1)
      p[2]='=';
    p+=4;
  }
  *p=0;
  *str = s;
  return strlen(s);
}

int base64_decode(const string& str, void* data)
{
	return base64_decode(str.c_str(), data);
}

int base64_decode(const char *str, void *data)
{
  const char *p;
  unsigned char *q;
  int c;
  int x;
  int done = 0;
  q=(unsigned char*)data;
  for(p=str; *p && !done; p+=4){
    x = pos(p[0]);
    if(x >= 0)
      c = x;
    else{
      done = 3;
      break;
    }
    c*=64;
    
    x = pos(p[1]);
    if(x >= 0)
      c += x;
    else
      return -1;
    c*=64;
    
    if(p[2] == '=')
      done++;
    else{
      x = pos(p[2]);
      if(x >= 0)
	c += x;
      else
	return -1;
    }
    c*=64;
    
    if(p[3] == '=')
      done++;
    else{
      if(done)
	return -1;
      x = pos(p[3]);
      if(x >= 0)
	c += x;
      else
	return -1;
    }
    if(done < 3)
      *q++=(c&0x00ff0000)>>16;
      
    if(done < 2)
      *q++=(c&0x0000ff00)>>8;
    if(done < 1)
      *q++=(c&0x000000ff)>>0;
  }
  return q - (unsigned char*)data;
}
