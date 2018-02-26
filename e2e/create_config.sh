#!/bin/bash
#

set -e

# expect GFUNC_HOST to contain the host for the function
# for example: us-central1-some-proj-id.cloudfunctions.net


# expect GFUNC_PATH to contain the host for the function
# for example: /function-1

if [ -z "$GFUNC_HOST" ]; then

FUNCTION=$(gcloud --format=json  functions list --limit 1 |jq -r '.[].httpsTrigger.url')

GFUNC_HOST=$(echo $FUNCTION | cut -d/ -f3)
GFUNC_PATH="/"$(echo $FUNCTION | cut -d/ -f4)

fi

cat > envoy.yaml << EOF
admin:
  access_log_path: /dev/stdout
  address:
    socket_address:
      address: 127.0.0.1
      port_value: 19000
static_resources:
  listeners:
  - name: listener_0
    address:
      socket_address: { address: 127.0.0.1, port_value: 10000 }
    filter_chains:
    - filters:
      - name: envoy.http_connection_manager
        config:
          stat_prefix: http
          codec_type: AUTO
          route_config:
            name: local_route
            virtual_hosts:
            - name: local_service
              domains: ["*"]
              routes:
              - match:
                  prefix: /googs
                route:
                  cluster: google-us-central1-proj1
                metadata:
                  filter_metadata:
                      io.solo.function_router:
                        google-us-central1-proj1:
                          function: uppercase
          http_filters:
          - name: io.solo.gcloudfunc
          - name: envoy.router
  clusters:
  - connect_timeout: 5.000s
    hosts:
    - socket_address:
        address: $GFUNC_HOST
        port_value: 443
    name: google-us-central1-proj1
    type: LOGICAL_DNS
    dns_lookup_family: V4_ONLY
    tls_context: {}
    metadata:
      filter_metadata:
        io.solo.function_router:
          functions:
            uppercase:
              path: $GFUNC_PATH
              host: $GFUNC_HOST
        io.solo.gcloudfunc: {}
EOF
