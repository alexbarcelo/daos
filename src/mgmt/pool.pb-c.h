/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: pool.proto */

#ifndef PROTOBUF_C_pool_2eproto__INCLUDED
#define PROTOBUF_C_pool_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _Mgmt__PoolCreateReq Mgmt__PoolCreateReq;
typedef struct _Mgmt__PoolCreateResp Mgmt__PoolCreateResp;
typedef struct _Mgmt__PoolDestroyReq Mgmt__PoolDestroyReq;
typedef struct _Mgmt__PoolDestroyResp Mgmt__PoolDestroyResp;
typedef struct _Mgmt__ListPoolsReq Mgmt__ListPoolsReq;
typedef struct _Mgmt__ListPoolsResp Mgmt__ListPoolsResp;
typedef struct _Mgmt__ListPoolsResp__Pool Mgmt__ListPoolsResp__Pool;


/* --- enums --- */


/* --- messages --- */

/*
 * PoolCreateReq supplies new pool parameters.
 */
struct  _Mgmt__PoolCreateReq
{
  ProtobufCMessage base;
  uint64_t scmbytes;
  uint64_t nvmebytes;
  /*
   * comma separated integers
   */
  char *ranks;
  /*
   * desired number of pool service replicas
   */
  uint32_t numsvcreps;
  /*
   * formatted user e.g. "bob@"
   */
  char *user;
  /*
   * formatted group e.g. "builders@"
   */
  char *usergroup;
  /*
   * UUID for new pool, generated on the client
   */
  char *uuid;
  /*
   * DAOS system identifier
   */
  char *sys;
  /*
   * Access Control Entries in short string format
   */
  size_t n_acl;
  char **acl;
};
#define MGMT__POOL_CREATE_REQ__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mgmt__pool_create_req__descriptor) \
    , 0, 0, (char *)protobuf_c_empty_string, 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0,NULL }


/*
 * PoolCreateResp returns created pool uuid and ranks.
 */
struct  _Mgmt__PoolCreateResp
{
  ProtobufCMessage base;
  /*
   * DAOS error code
   */
  int32_t status;
  /*
   * comma separated integers
   */
  char *svcreps;
};
#define MGMT__POOL_CREATE_RESP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mgmt__pool_create_resp__descriptor) \
    , 0, (char *)protobuf_c_empty_string }


/*
 * PoolDestroyReq supplies pool identifier and force flag.
 */
struct  _Mgmt__PoolDestroyReq
{
  ProtobufCMessage base;
  /*
   * uuid of pool to destroy
   */
  char *uuid;
  /*
   * DAOS system identifier
   */
  char *sys;
  /*
   * destroy regardless of active connections
   */
  protobuf_c_boolean force;
};
#define MGMT__POOL_DESTROY_REQ__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mgmt__pool_destroy_req__descriptor) \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0 }


/*
 * PoolDestroyResp returns resultant state of destroy operation.
 */
struct  _Mgmt__PoolDestroyResp
{
  ProtobufCMessage base;
  /*
   * DAOS error code
   */
  int32_t status;
};
#define MGMT__POOL_DESTROY_RESP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mgmt__pool_destroy_resp__descriptor) \
    , 0 }


struct  _Mgmt__ListPoolsReq
{
  ProtobufCMessage base;
  /*
   * DAOS system identifier
   */
  char *sys;
  /*
   * Client response buffer capacity in number of pools
   */
  uint64_t numpools;
};
#define MGMT__LIST_POOLS_REQ__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mgmt__list_pools_req__descriptor) \
    , (char *)protobuf_c_empty_string, 0 }


struct  _Mgmt__ListPoolsResp__Pool
{
  ProtobufCMessage base;
  /*
   * uuid of pool
   */
  char *uuid;
  /*
   * pool service replicas, comma-separated integers
   */
  char *svcreps;
};
#define MGMT__LIST_POOLS_RESP__POOL__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mgmt__list_pools_resp__pool__descriptor) \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string }


struct  _Mgmt__ListPoolsResp
{
  ProtobufCMessage base;
  /*
   * DAOS error code
   */
  int32_t status;
  /*
   * pools list (max length ListPoolsReq.numPools)
   */
  size_t n_pools;
  Mgmt__ListPoolsResp__Pool **pools;
  /*
   * number of pools in system
   */
  uint64_t numpools;
};
#define MGMT__LIST_POOLS_RESP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&mgmt__list_pools_resp__descriptor) \
    , 0, 0,NULL, 0 }


/* Mgmt__PoolCreateReq methods */
void   mgmt__pool_create_req__init
                     (Mgmt__PoolCreateReq         *message);
size_t mgmt__pool_create_req__get_packed_size
                     (const Mgmt__PoolCreateReq   *message);
size_t mgmt__pool_create_req__pack
                     (const Mgmt__PoolCreateReq   *message,
                      uint8_t             *out);
size_t mgmt__pool_create_req__pack_to_buffer
                     (const Mgmt__PoolCreateReq   *message,
                      ProtobufCBuffer     *buffer);
Mgmt__PoolCreateReq *
       mgmt__pool_create_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mgmt__pool_create_req__free_unpacked
                     (Mgmt__PoolCreateReq *message,
                      ProtobufCAllocator *allocator);
/* Mgmt__PoolCreateResp methods */
void   mgmt__pool_create_resp__init
                     (Mgmt__PoolCreateResp         *message);
size_t mgmt__pool_create_resp__get_packed_size
                     (const Mgmt__PoolCreateResp   *message);
size_t mgmt__pool_create_resp__pack
                     (const Mgmt__PoolCreateResp   *message,
                      uint8_t             *out);
size_t mgmt__pool_create_resp__pack_to_buffer
                     (const Mgmt__PoolCreateResp   *message,
                      ProtobufCBuffer     *buffer);
Mgmt__PoolCreateResp *
       mgmt__pool_create_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mgmt__pool_create_resp__free_unpacked
                     (Mgmt__PoolCreateResp *message,
                      ProtobufCAllocator *allocator);
/* Mgmt__PoolDestroyReq methods */
void   mgmt__pool_destroy_req__init
                     (Mgmt__PoolDestroyReq         *message);
size_t mgmt__pool_destroy_req__get_packed_size
                     (const Mgmt__PoolDestroyReq   *message);
size_t mgmt__pool_destroy_req__pack
                     (const Mgmt__PoolDestroyReq   *message,
                      uint8_t             *out);
size_t mgmt__pool_destroy_req__pack_to_buffer
                     (const Mgmt__PoolDestroyReq   *message,
                      ProtobufCBuffer     *buffer);
Mgmt__PoolDestroyReq *
       mgmt__pool_destroy_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mgmt__pool_destroy_req__free_unpacked
                     (Mgmt__PoolDestroyReq *message,
                      ProtobufCAllocator *allocator);
/* Mgmt__PoolDestroyResp methods */
void   mgmt__pool_destroy_resp__init
                     (Mgmt__PoolDestroyResp         *message);
size_t mgmt__pool_destroy_resp__get_packed_size
                     (const Mgmt__PoolDestroyResp   *message);
size_t mgmt__pool_destroy_resp__pack
                     (const Mgmt__PoolDestroyResp   *message,
                      uint8_t             *out);
size_t mgmt__pool_destroy_resp__pack_to_buffer
                     (const Mgmt__PoolDestroyResp   *message,
                      ProtobufCBuffer     *buffer);
Mgmt__PoolDestroyResp *
       mgmt__pool_destroy_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mgmt__pool_destroy_resp__free_unpacked
                     (Mgmt__PoolDestroyResp *message,
                      ProtobufCAllocator *allocator);
/* Mgmt__ListPoolsReq methods */
void   mgmt__list_pools_req__init
                     (Mgmt__ListPoolsReq         *message);
size_t mgmt__list_pools_req__get_packed_size
                     (const Mgmt__ListPoolsReq   *message);
size_t mgmt__list_pools_req__pack
                     (const Mgmt__ListPoolsReq   *message,
                      uint8_t             *out);
size_t mgmt__list_pools_req__pack_to_buffer
                     (const Mgmt__ListPoolsReq   *message,
                      ProtobufCBuffer     *buffer);
Mgmt__ListPoolsReq *
       mgmt__list_pools_req__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mgmt__list_pools_req__free_unpacked
                     (Mgmt__ListPoolsReq *message,
                      ProtobufCAllocator *allocator);
/* Mgmt__ListPoolsResp__Pool methods */
void   mgmt__list_pools_resp__pool__init
                     (Mgmt__ListPoolsResp__Pool         *message);
/* Mgmt__ListPoolsResp methods */
void   mgmt__list_pools_resp__init
                     (Mgmt__ListPoolsResp         *message);
size_t mgmt__list_pools_resp__get_packed_size
                     (const Mgmt__ListPoolsResp   *message);
size_t mgmt__list_pools_resp__pack
                     (const Mgmt__ListPoolsResp   *message,
                      uint8_t             *out);
size_t mgmt__list_pools_resp__pack_to_buffer
                     (const Mgmt__ListPoolsResp   *message,
                      ProtobufCBuffer     *buffer);
Mgmt__ListPoolsResp *
       mgmt__list_pools_resp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   mgmt__list_pools_resp__free_unpacked
                     (Mgmt__ListPoolsResp *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Mgmt__PoolCreateReq_Closure)
                 (const Mgmt__PoolCreateReq *message,
                  void *closure_data);
typedef void (*Mgmt__PoolCreateResp_Closure)
                 (const Mgmt__PoolCreateResp *message,
                  void *closure_data);
typedef void (*Mgmt__PoolDestroyReq_Closure)
                 (const Mgmt__PoolDestroyReq *message,
                  void *closure_data);
typedef void (*Mgmt__PoolDestroyResp_Closure)
                 (const Mgmt__PoolDestroyResp *message,
                  void *closure_data);
typedef void (*Mgmt__ListPoolsReq_Closure)
                 (const Mgmt__ListPoolsReq *message,
                  void *closure_data);
typedef void (*Mgmt__ListPoolsResp__Pool_Closure)
                 (const Mgmt__ListPoolsResp__Pool *message,
                  void *closure_data);
typedef void (*Mgmt__ListPoolsResp_Closure)
                 (const Mgmt__ListPoolsResp *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor mgmt__pool_create_req__descriptor;
extern const ProtobufCMessageDescriptor mgmt__pool_create_resp__descriptor;
extern const ProtobufCMessageDescriptor mgmt__pool_destroy_req__descriptor;
extern const ProtobufCMessageDescriptor mgmt__pool_destroy_resp__descriptor;
extern const ProtobufCMessageDescriptor mgmt__list_pools_req__descriptor;
extern const ProtobufCMessageDescriptor mgmt__list_pools_resp__descriptor;
extern const ProtobufCMessageDescriptor mgmt__list_pools_resp__pool__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_pool_2eproto__INCLUDED */
