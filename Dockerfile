FROM alpine:latest AS builder

RUN apk add --no-cache build-base cmake

WORKDIR /app
COPY CMakeLists.txt .
COPY src/ ./src/

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j $(nproc)

FROM alpine:latest

RUN apk add --no-cache libstdc++

WORKDIR /app
COPY --from=builder /app/build/web_server .
COPY static/ ./static/

WORKDIR /app/static

EXPOSE 80

ENTRYPOINT ["/app/web_server"]