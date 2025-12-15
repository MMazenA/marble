#vcpkg 
git submodule update --init --recursive

# docker setup
# http tests nginx cert
if [ ! -f config/nginx/ssl/server.crt ]; then
  mkdir -p config/nginx/ssl
  openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
    -keyout config/nginx/ssl/server.key \
    -out config/nginx/ssl/server.crt \
    -subj "/CN=localhost" \
    -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
fi

docker compose up -d


#kafka setup
#idk rn lol


# executables
rm -rf build
cmake --preset dev
cmake --build --preset dev --target all
