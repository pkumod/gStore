add_library(gstore_grpc OBJECT
        APIUtil.cpp
        grpc_status_code.cpp
        grpc_multipart_parser.cpp
        grpc_content.cpp
        grpc_message.cpp
        grpc_server_task.cpp
        grpc_routetable.cpp
        grpc_router.cpp
        grpc_server.cpp
)