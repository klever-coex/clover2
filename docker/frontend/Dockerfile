FROM node:20-alpine AS builder

WORKDIR /app
COPY frontend .

RUN npm ci
RUN npm run build

FROM node:20-alpine AS runner

ENV HOSTNAME=0.0.0.0
ENV PORT=80

WORKDIR /app

COPY --from=builder /app/.next/standalone ./

COPY --from=builder /app/.next/static ./.next/static
COPY --from=builder /app/public ./public

COPY docker/frontend/entrypoint.sh .
RUN chmod +x entrypoint.sh

CMD ["./entrypoint.sh"]
