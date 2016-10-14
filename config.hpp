/*
Copyright (c) 2009, Madis Kaal <mast@nomad.ee> 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the involved organizations nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __config_hpp__
#define __config_hpp__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "thread.hpp"

// configuration loader and manager
//
// entire config file is loaded into memory as linked list
// user of config is responsible for Lock() and Unlock() calls
// to ensure that other threads do not modify config settings unexpectedly
//
// config setting names are case insensitive
//
class CConfig {
  // one for each line of config
  typedef struct _citem {
    struct _citem *next;
    char *name;
    char *value;
    char *comment;
  } CITEM;
  // one for each config change listener
  typedef struct _clistener {
    struct _clistener *next;
    char *name;
    void (*callback)(const char *name,const char *value);
  } CLISTENER;
    
  CITEM *items;
  CLISTENER *listeners;
  
  CMutex mutex,listenermutex;
  
  // each config change is broadcast to all interested listeners
  void BroadcastConfigChange(char *name,char *value)
  {
    listenermutex.Lock();
    CLISTENER *l=listeners;
    while (l) {
      if (!l->name || !strcasecmp(l->name,name))
        (*l->callback)(name,value);
      l=l->next;
    }
    listenermutex.Unlock();
  }

    
public:
  
  char *skipspace(char *s)
  {
    while (s && *s && isspace(*s))
      s++;
    return s;
  }
  
  char *rtrim(char *s)
  {
    char *t=strchr(s,'\0')-1;
    while (t>=s && isspace(*t))
      *t--='\0';
    return s;
  }

  char *trim(char *s)
  {
    s=skipspace(s);
    return rtrim(s);
  }

  CConfig()
  {
    items=NULL;
    listeners=NULL;
  }
  
  ~CConfig()
  {
    Clear();
  }
  
  void Lock() { mutex.Lock(); }
  void Unlock() { mutex.Unlock(); }
  
  // register new listener for config entry given by name
  // register for NULL name to listen for all config changes
  // returns false if fails to add listener
  //
  bool RegisterListener(char *name,void (*cb)(const char *n,const char *v))
  {
    CLISTENER *l;
    listenermutex.Lock();
    l=new CLISTENER;
    if (l) {
      if (name) {
        l->name=new char[strlen(name)+1];
        if (l->name) {
          strcpy(l->name,name);
        }
        else {
          delete l;
          listenermutex.Unlock();
          return false;
        }
      }
      else
        l->name=NULL;
      l->callback=cb;
      l->next=listeners;
      listeners=l;
      listenermutex.Unlock();
      return true;
    }
    listenermutex.Unlock();
    return false;
  }
  
  // discards all items and listeners
  //
  void Clear()
  {
    mutex.Lock();
    CITEM *t=items;
    while (t) {
      items=t->next;
      if (t->name)
        delete t->name;
      if (t->value)
        delete t->value;
      if (t->comment)
        delete t->comment;
      delete t;
      t=items;
    }
    items=NULL;
    mutex.Unlock();
    listenermutex.Lock();
    CLISTENER *l=listeners;
    while (l) {
      listeners=l->next;
      if (l->name)
        delete l->name;
      delete l;
      l=listeners;
    }
    listeners=NULL;
    listenermutex.Unlock();
  }
  
  // gets pointer to value
  // remember that the pointer may change if SetValue is called
  //
  char *GetValue(char *name)
  {
    CITEM *t=items;
    while (name && t) {
      if (t->name && !strcasecmp(t->name,name))
        return t->value;
      t=t->next;
    }
    return NULL;
  }

  // add or modify a value of setting
  // either case causes interested listeners to be notified
  // modification of existing value always causes reallocation
  // of value buffer
  //
  char *SetValue(char *name,char *value)
  {
    Lock();
    CITEM *t=items;
    while (name && t) {
      if (t->name && !strcasecmp(t->name,name)) {
        if (t->value)
          delete t->value;
        t->value=new char[strlen(value)+1];
        if (t->value)
          strcpy(t->value,value);
        BroadcastConfigChange(name,value);
        Unlock();
        return t->value;
      }
      if (!t->next) {
        t->next=new CITEM;
        if (t->next) {
          t->next->next=NULL;
          t->next->comment=NULL;
          t->next->name=new char[strlen(name)+1];
          if (t->next->name) {
            strcpy(t->next->name,name);
            t->next->value=new char[strlen(value)+1];
            if (t->next->value) {
              strcpy(t->next->value,value);
              BroadcastConfigChange(name,value);
              Unlock();
              return t->next->value;
            }
            delete t->next->name;
          }
          delete t->next;
          t->next=NULL;
        }
        Unlock();
        return NULL;
      }
      t=t->next;
    }
    Unlock();
    return NULL;
  }
  
  // helper functions to convert settings to appropriate type
  // allowing the default value to be specified for nonexistant settings
  //
  bool GetBoolSetting(char *name,bool def=false)
  {
    char *s=GetValue(name);
    if (s && (*s=='y' || *s=='Y' || *s=='t' || *s=='T' || *s=='1'))
      return true;
    return s?false:def;
  }
  
  int GetIntSetting(char *name,int def=0)
  {
    char *s=GetValue(name);
    if (!s)
      return def;
    return atoi(s);
  }
  
  const char *GetStringSetting(char *name,const char *def=NULL)
  {
    char *s=GetValue(name);
    if (!s)
      return def;
    return (const char *)s;
  }

  double GetDoubleSetting(char *name,double def=0)
  {
    char *s=GetValue(name);
    if (!s)
      return def;
    return strtod(s,NULL);
  }
  
  // save config to file, comments are preserved but formatting is lost
  //
  bool Save(char *filename)
  {
    FILE *fp=fopen(filename,"wt");
    CITEM *t=items;
    if (fp) {
      while (t) {
        if (t->name) {
          if (!strchr(t->value,'#'))
            fprintf(fp,"%s=%s",t->name,t->value);
          else {
            fprintf(fp,"%s=",t->name);
            char *s=t->value;
            while (s && *s) {
              if (*s=='#')
                fprintf(fp,"\\#");
              else
                fprintf(fp,"%c",*s);
              s++;
            }
          }
        }
        if (t->comment)
          fprintf(fp,"%s#%s\n",t->name?" ":"",t->comment);
        else
          fprintf(fp,"\n");
        t=t->next;
      }
      fclose(fp);
      return true;
    }
    return false;
  }
  
  // loads config from file
  // config file syntax is
  //   name=value [# comment]
  // parser splits lines at '#' then splits the left side 
  // line at '=' and strips whitespace in front and and 
  // end of each half. values are case sensitive but names 
  // are not. if you need to include '#' in text, put a 
  // backslash in front of it
  //
  bool Load(char *filename)
  {
    Clear();
    CITEM* ilist=NULL;
    FILE *fp=fopen(filename,"rt");
    char l[1024],*t,*comment=NULL;
    if (fp) {
      while (!feof(fp) && !ferror(fp)) {
        if (fgets(l,sizeof l-1,fp)) {
          l[sizeof(l)-1]='\0';
          t=l;
          comment=NULL;
          while ((t=strchr(t,'#'))!=NULL) {
            if (t==l || *(t-1)!='\\') {
              *t='\0';
              comment=t+1;
              break;
            }
            strcpy(t-1,t);
          }
          t=strchr(l,'=');
          if (t || comment) {
            CITEM *item=new CITEM;
            if (item) {
              if (comment) {
                rtrim(comment);
                item->comment=new char[strlen(comment)+1];
                if (item->comment)
                  strcpy(item->comment,comment);
              }
              else
                item->comment=NULL;
              t=strchr(l,'=');
              if (t) {
                *t='\0';
                t=trim(t+1);
                item->value=new char[strlen(t)+1];
                if (item->value)
                  strcpy(item->value,t);
                t=trim(l);
                item->name=new char[strlen(t)+1];
                if (item->name)
                  strcpy(item->name,t);
              }
              else {
                item->name=NULL;
                item->value=NULL;
              }
              if (!items) {
                items=item;
                ilist=item;
              }
              else {
                ilist->next=item;
                ilist=item;
              }
            }
          }
        }
      }
      fclose(fp);
    }
    return fp!=NULL;
  }
  
};

#endif
