type Token = number | string;

const NUMBER_END = /[0-9.]/;

function tokenize(src: string): Token[] {
  const tokens: Token[] = [];
  let i = 0;

  while (i < src.length) {
    const ch = src[i]!;

    if (ch === ' ' || ch === '\t') {
      i++;
    } else if (NUMBER_END.test(ch)) {
      let j = i + 1;
      while (j < src.length && NUMBER_END.test(src[j]!)) j++;
      if ((src[j] === 'e' || src[j] === 'E') && /[0-9]/.test(src[j + 1] ?? '')) {
        j += 2;
        while (j < src.length && /[0-9]/.test(src[j]!)) j++;
      }
      const num = Number(src.slice(i, j));
      if (!Number.isFinite(num)) throw new Error('Invalid expression');
      tokens.push(num);
      i = j;
    } else if (ch === '*' && src[i + 1] === '*') {
      tokens.push('**');
      i += 2;
    } else if (ch === '/' && src[i + 1] === '/') {
      tokens.push('//');
      i += 2;
    } else if ('+-*/%()'.includes(ch)) {
      tokens.push(ch);
      i++;
    } else {
      throw new Error('Invalid expression');
    }
  }

  return tokens;
}

export function evaluateExpr(expr: string, vars: { id: number }): number {
  const substituted = expr.trim().replaceAll(/\bid\b/g, String(vars.id));
  if (!substituted) throw new Error('Invalid expression');

  const tokens = tokenize(substituted);
  if (tokens.length === 0) throw new Error('Invalid expression');

  let pos = 0;
  const peek = (): Token | undefined => tokens[pos];
  const next = (): Token => {
    const t = tokens[pos++];
    if (t === undefined) throw new Error('Invalid expression');
    return t;
  };

  // sum := term (('+' | '-') term)*
  const sum = (): number => {
    let left = term();
    while (peek() === '+' || peek() === '-') {
      left = next() === '+' ? left + term() : left - term();
    }
    return left;
  };

  // term := unary (('*' | '/' | '//' | '%') unary)*  — left-associative
  const term = (): number => {
    let left = unary();
    for (;;) {
      if (peek() === '*') {
        next();
        left *= unary();
      } else if (peek() === '/') {
        next();
        left /= unary();
      } else if (peek() === '//') {
        next();
        left = Math.floor(left / unary());
      } else if (peek() === '%') {
        next();
        const right = unary();
        left = ((left % right) + right) % right; // python modulo
      } else {
        return left;
      }
    }
  };

  // unary := ('-' | '+') unary | power
  const unary = (): number => {
    if (peek() === '-') {
      next();
      return -unary();
    }
    if (peek() === '+') {
      next();
      return unary();
    }
    return power();
  };

  // power := primary ('**' unary)?  — right-associative via recursion
  const power = (): number => {
    const base = primary();
    if (peek() === '**') {
      next();
      return base ** unary();
    }
    return base;
  };

  const primary = (): number => {
    const t = next();
    if (typeof t === 'number') return t;
    if (t === '(') {
      const v = sum();
      if (next() !== ')') throw new Error('Invalid expression');
      return v;
    }
    throw new Error('Invalid expression');
  };

  const result = sum();
  if (pos !== tokens.length) throw new Error('Invalid expression');
  if (!Number.isFinite(result)) throw new Error('Result is not finite');
  return result;
}
