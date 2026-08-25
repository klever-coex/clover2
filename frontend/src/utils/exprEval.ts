type Token =
  | { type: 'NUM'; value: number }
  | { type: 'VAR'; name: string }
  | { type: 'OP'; op: string }
  | { type: 'LPAREN' }
  | { type: 'RPAREN' };

const OP_CHARS = new Set(['+', '-', '*', '/', '%', '(', ')']);

function tokenize(expr: string): Token[] {
  const tokens: Token[] = [];
  let i = 0;

  while (i < expr.length) {
    const ch = expr[i]!;

    // Whitespace
    if (ch === ' ') {
      i++;
      continue;
    }

    if (ch === '(') {
      tokens.push({ type: 'LPAREN' });
      i++;
      continue;
    }
    if (ch === ')') {
      tokens.push({ type: 'RPAREN' });
      i++;
      continue;
    }

    if (ch === '*' && expr[i + 1] === '*') {
      tokens.push({ type: 'OP', op: '**' });
      i += 2;
      continue;
    }

    if (ch === '/' && expr[i + 1] === '/') {
      tokens.push({ type: 'OP', op: '//' });
      i += 2;
      continue;
    }

    if (OP_CHARS.has(ch)) {
      tokens.push({ type: 'OP', op: ch });
      i++;
      continue;
    }

    if (/[0-9]/.test(ch) || (ch === '.' && i + 1 < expr.length && /[0-9]/.test(expr[i + 1]!))) {
      let num = '';
      while (i < expr.length) {
        const c = expr[i]!;
        if (/[0-9.eE]/.test(c)) {
          num += c;
          i++;
        } else if ((c === '+' || c === '-') && num.length > 0 && /[eE]/.test(num[num.length - 1]!)) {
          num += c;
          i++;
        } else {
          break;
        }
      }
      const val = parseFloat(num);
      if (isNaN(val)) throw new Error(`Invalid number: "${num}"`);
      tokens.push({ type: 'NUM', value: val });
      continue;
    }

    if (/[a-zA-Z_]/.test(ch)) {
      let name = '';
      while (i < expr.length && /[a-zA-Z0-9_]/.test(expr[i]!)) {
        name += expr[i];
        i++;
      }
      tokens.push({ type: 'VAR', name });
      continue;
    }

    throw new Error(`Unexpected character: "${ch}" at position ${i}`);
  }

  return tokens;
}

class Parser {
  private tokens: Token[];
  private pos: number;

  constructor(tokens: Token[]) {
    this.tokens = tokens;
    this.pos = 0;
  }

  peek(): Token | undefined {
    return this.tokens[this.pos];
  }

  consume(): Token {
    const t = this.tokens[this.pos++];
    if (!t) throw new Error('Unexpected end of expression');
    return t;
  }

  expect(type: Token['type'], value?: string): Token {
    const t = this.consume();
    if (t.type !== type) {
      throw new Error(`Expected ${type}, got ${t.type}`);
    }
    if (value !== undefined && t.type === 'OP' && (t as { op: string }).op !== value) {
      throw new Error(`Expected "${value}", got "${(t as { op: string }).op}"`);
    }
    return t;
  }

  expr(vars: Record<string, number>): number {
    let left = this.term(vars);

    while (this.peek()?.type === 'OP' && ((this.peek() as { op: string }).op === '+' || (this.peek() as { op: string }).op === '-')) {
      const op = (this.consume() as { op: string }).op;
      const right = this.term(vars);
      left = op === '+' ? left + right : left - right;
    }

    return left;
  }

  private term(vars: Record<string, number>): number {
    let left = this.power(vars);

    while (this.peek()?.type === 'OP' && ['*', '/', '//', '%'].includes((this.peek() as { op: string }).op)) {
      const op = (this.consume() as { op: string }).op;
      const right = this.power(vars);
      if (right === 0) throw new Error('Division by zero');
      if (op === '*') left = left * right;
      else if (op === '/') left = left / right;
      else if (op === '//') left = Math.trunc(left / right);
      else left = left % right;
    }

    return left;
  }

  private power(vars: Record<string, number>): number {
    const base = this.unary(vars);
    if (this.peek()?.type === 'OP' && (this.peek() as { op: string }).op === '**') {
      this.consume();
      const exp = this.power(vars);
      return base ** exp;
    }
    return base;
  }

  private unary(vars: Record<string, number>): number {
    if (this.peek()?.type === 'OP' && ((this.peek() as { op: string }).op === '+' || (this.peek() as { op: string }).op === '-')) {
      const op = (this.consume() as { op: string }).op;
      const v = this.unary(vars);
      return op === '-' ? -v : v;
    }
    return this.primary(vars);
  }

  private primary(vars: Record<string, number>): number {
    const t = this.peek();
    if (!t) throw new Error('Unexpected end of expression');

    if (t.type === 'NUM') {
      this.consume();
      return (t as { value: number }).value;
    }

    if (t.type === 'VAR') {
      this.consume();
      const name = (t as { name: string }).name;
      if (!(name in vars)) throw new Error(`Unknown variable: "${name}"`);
      return vars[name]!;
    }

    if (t.type === 'LPAREN') {
      this.consume();
      const v = this.expr(vars);
      this.expect('RPAREN');
      return v;
    }

    throw new Error(`Unexpected token: ${t.type}`);
  }
}

/**
 * Evaluate an arithmetic expression with variable substitution.
 *
 * @param expr  Expression string (e.g. "id * 1500 + 100")
 * @param vars  Variable name value map (e.g. { id: 3 })
 * @returns     Numeric result
 * @throws      On parse or evaluation errors
 */
export function evaluateExpr(expr: string, vars: Record<string, number>): number {
  const trimmed = expr.trim();
  if (!trimmed) throw new Error('Empty expression');

  const tokens = tokenize(trimmed);
  if (tokens.length === 0) throw new Error('Empty expression');

  const parser = new Parser(tokens);
  const result = parser.expr(vars);

  if (parser.peek() !== undefined) {
    throw new Error('Unexpected tokens after expression');
  }

  if (!Number.isFinite(result)) {
    throw new Error(`Result is not a finite number: ${result}`);
  }

  return result;
}
