FROM python:3.11-slim AS builder

WORKDIR /app

# COPY requirements.txt .
# COPY mkdocs.yml .
COPY . /app

RUN pip install --no-cache-dir -r requirements.txt && \
    mkdocs build --site-dir /tmp/site

FROM nginx:1.25-alpine

COPY --from=builder /tmp/site /usr/share/nginx/html

COPY nginx.conf /etc/nginx/nginx.conf

EXPOSE 80

CMD ["nginx", "-g", "daemon off;"]