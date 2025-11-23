# ---------- Build stage ----------
FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libpq-dev \
    libssl-dev \
    nlohmann-json3-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN rm -rf build

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --config Release -j"$(nproc)"

# ---------- Runtime stage ----------
FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y \
    libpq5 \
    libssl3 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=build /app/build/user_registry /app/user_registry
COPY --from=build /app/build/gen_users   /app/gen_users

EXPOSE 8080

CMD ["./user_registry"]

