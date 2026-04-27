FROM node:20-alpine AS builder

WORKDIR /app
COPY frontend .

RUN npm ci
RUN npm run build

FROM node:20-alpine AS runner

WORKDIR /app

COPY --from=builder /app/.next/standalone ./

COPY --from=builder /app/.next/static ./.next/static
COPY --from=builder /app/public ./public

EXPOSE 80

CMD ["node", "server.js", "--hostname", "0.0.0.0", "--port", "80"]
