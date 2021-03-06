/* soapStub.h
   Generated by gSOAP 2.8.3 from plasma.h

Copyright(C) 2000-2011, Robert van Engelen, Genivia Inc. All Rights Reserved.
The generated code is released under one of the following licenses:
1) GPL or 2) Genivia's license for commercial use.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
*/

#ifndef soapStub_H
#define soapStub_H
#include <vector>
#define SOAP_NAMESPACE_OF_Plasma	"http://tempuri.org/"
#include "stdsoap2.h"

/******************************************************************************\
 *                                                                            *
 * Enumerations                                                               *
 *                                                                            *
\******************************************************************************/


#ifndef SOAP_TYPE_Plasma__ResourceType
#define SOAP_TYPE_Plasma__ResourceType (16)
/* Plasma:ResourceType */
enum Plasma__ResourceType {Plasma__ResourceType__Map = 0, Plasma__ResourceType__Mod = 1};
#endif

/******************************************************************************\
 *                                                                            *
 * Types with Custom Serializers                                              *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * Classes and Structs                                                        *
 *                                                                            *
\******************************************************************************/


#if 0 /* volatile type: do not declare here, declared elsewhere */

#endif

#ifndef SOAP_TYPE_xsd__base64Binary
#define SOAP_TYPE_xsd__base64Binary (8)
/* Base64 schema type: */
class SOAP_CMAC xsd__base64Binary
{
public:
	unsigned char *__ptr;
	int __size;
	char *id;	/* optional element of type xsd:string */
	char *type;	/* optional element of type xsd:string */
	char *options;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 8; } /* = unique id SOAP_TYPE_xsd__base64Binary */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         xsd__base64Binary() { xsd__base64Binary::soap_default(NULL); }
	virtual ~xsd__base64Binary() { }
};
#endif

#if 0 /* volatile type: do not declare here, declared elsewhere */

#endif

#ifndef SOAP_TYPE_Plasma__ArrayOfString
#define SOAP_TYPE_Plasma__ArrayOfString (13)
/* Plasma:ArrayOfString */
class SOAP_CMAC Plasma__ArrayOfString
{
public:
	std::vector<std::string >string;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 13; } /* = unique id SOAP_TYPE_Plasma__ArrayOfString */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         Plasma__ArrayOfString() { Plasma__ArrayOfString::soap_default(NULL); }
	virtual ~Plasma__ArrayOfString() { }
};
#endif

#ifndef SOAP_TYPE__Plasma__DownloadFile
#define SOAP_TYPE__Plasma__DownloadFile (14)
/* Plasma:DownloadFile */
class SOAP_CMAC _Plasma__DownloadFile
{
public:
	std::string *internalName;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 14; } /* = unique id SOAP_TYPE__Plasma__DownloadFile */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         _Plasma__DownloadFile() { _Plasma__DownloadFile::soap_default(NULL); }
	virtual ~_Plasma__DownloadFile() { }
};
#endif

#ifndef SOAP_TYPE__Plasma__DownloadFileResponse
#define SOAP_TYPE__Plasma__DownloadFileResponse (15)
/* Plasma:DownloadFileResponse */
class SOAP_CMAC _Plasma__DownloadFileResponse
{
public:
	bool DownloadFileResult;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* required element of type xsd:boolean */
	Plasma__ArrayOfString *links;	/* optional element of type Plasma:ArrayOfString */
	xsd__base64Binary *torrent;	/* optional element of type xsd:base64Binary */
	Plasma__ArrayOfString *dependencies;	/* optional element of type Plasma:ArrayOfString */
	enum Plasma__ResourceType resourceType;	/* required element of type Plasma:ResourceType */
	std::string *torrentFileName;	/* optional element of type xsd:string */
	struct soap *soap;	/* transient */
public:
	virtual int soap_type() const { return 15; } /* = unique id SOAP_TYPE__Plasma__DownloadFileResponse */
	virtual void soap_default(struct soap*);
	virtual void soap_serialize(struct soap*) const;
	virtual int soap_put(struct soap*, const char*, const char*) const;
	virtual int soap_out(struct soap*, const char*, int, const char*) const;
	virtual void *soap_get(struct soap*, const char*, const char*);
	virtual void *soap_in(struct soap*, const char*, const char*);
	         _Plasma__DownloadFileResponse() { _Plasma__DownloadFileResponse::soap_default(NULL); }
	virtual ~_Plasma__DownloadFileResponse() { }
};
#endif

#ifndef SOAP_TYPE___Plasma__DownloadFile
#define SOAP_TYPE___Plasma__DownloadFile (26)
/* Operation wrapper: */
struct __Plasma__DownloadFile
{
public:
	_Plasma__DownloadFile *Plasma__DownloadFile;	/* optional element of type Plasma:DownloadFile */
};
#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Header
#define SOAP_TYPE_SOAP_ENV__Header (27)
/* SOAP Header: */
struct SOAP_ENV__Header
{
#ifdef WITH_NOEMPTYSTRUCT
private:
	char dummy;	/* dummy member to enable compilation */
#endif
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Code
#define SOAP_TYPE_SOAP_ENV__Code (28)
/* SOAP Fault Code: */
struct SOAP_ENV__Code
{
public:
	char *SOAP_ENV__Value;	/* optional element of type xsd:QName */
	struct SOAP_ENV__Code *SOAP_ENV__Subcode;	/* optional element of type SOAP-ENV:Code */
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Detail
#define SOAP_TYPE_SOAP_ENV__Detail (30)
/* SOAP-ENV:Detail */
struct SOAP_ENV__Detail
{
public:
	char *__any;
	int __type;	/* any type of element <fault> (defined below) */
	void *fault;	/* transient */
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Reason
#define SOAP_TYPE_SOAP_ENV__Reason (33)
/* SOAP-ENV:Reason */
struct SOAP_ENV__Reason
{
public:
	char *SOAP_ENV__Text;	/* optional element of type xsd:string */
};
#endif

#endif

#ifndef WITH_NOGLOBAL

#ifndef SOAP_TYPE_SOAP_ENV__Fault
#define SOAP_TYPE_SOAP_ENV__Fault (34)
/* SOAP Fault: */
struct SOAP_ENV__Fault
{
public:
	char *faultcode;	/* optional element of type xsd:QName */
	char *faultstring;	/* optional element of type xsd:string */
	char *faultactor;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *detail;	/* optional element of type SOAP-ENV:Detail */
	struct SOAP_ENV__Code *SOAP_ENV__Code;	/* optional element of type SOAP-ENV:Code */
	struct SOAP_ENV__Reason *SOAP_ENV__Reason;	/* optional element of type SOAP-ENV:Reason */
	char *SOAP_ENV__Node;	/* optional element of type xsd:string */
	char *SOAP_ENV__Role;	/* optional element of type xsd:string */
	struct SOAP_ENV__Detail *SOAP_ENV__Detail;	/* optional element of type SOAP-ENV:Detail */
};
#endif

#endif

/******************************************************************************\
 *                                                                            *
 * Typedefs                                                                   *
 *                                                                            *
\******************************************************************************/

#ifndef SOAP_TYPE__QName
#define SOAP_TYPE__QName (5)
typedef char *_QName;
#endif

#ifndef SOAP_TYPE__XML
#define SOAP_TYPE__XML (6)
typedef char *_XML;
#endif


/******************************************************************************\
 *                                                                            *
 * Externals                                                                  *
 *                                                                            *
\******************************************************************************/


#endif

/* End of soapStub.h */
