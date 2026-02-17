FROM node:20-alpine AS builder

WORKDIR /app
COPY frontend .

RUN npm ci
RUN npm run build

FROM nginx:1.29.3-alpine-slim

COPY --from=builder /app/dist /usr/share/nginx/html

COPY docker/frontend/nginx.conf /etc/nginx/conf.d/default.conf

CMD ["nginx", "-g", "daemon off;"]
