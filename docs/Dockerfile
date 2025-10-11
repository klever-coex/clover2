FROM python:3.11-slim AS builder
# FROM alpine:3.14 AS builder

RUN apt-get update && \
    apt-get install -y make && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

RUN pip3 install --no-cache-dir -r requirements.txt

RUN make html 
# sphinx-build -M html . /tmp/site

FROM nginx:1.25-alpine

COPY --from=builder /app/build/html /usr/share/nginx/html

COPY nginx.conf /etc/nginx/nginx.conf

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]