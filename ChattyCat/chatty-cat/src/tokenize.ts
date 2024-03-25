export enum TokenType {
  word,
  string,
  regexp,
  number,
  punctuator,
  end
}

export type Token = {
  type: TokenType;
  value: string;
  line: number;
  col: number;
};

export function* tokenize(source: string) {
  let line = 1;
  let col = 1;
  let i = 0;
  while (i < source.length) {
    let c = source[i];
    if (isspace(c)) {
      // 跳过空白字符
      if (c === '\n') {
        line++;
        col = 1;
      } else {
        col++;
      }
      i++;
    } else if (c === '#') {
      // 跳过注释
      let j = i + 1;
      while (j < source.length && source[j] !== '\n') {
        j++;
      }
      i = j;
      col = 1;
    } else if (isalpha(c) || c === '$' || c === '_') {
      // 识别单词（关键字和标识符）
      let j = i + 1;
      while (j < source.length && (isalnum(source[j]) || source[j] === '_')) {
        j++;
      }
      let value = source.substring(i, j);
      yield { type: TokenType.word, value, line, col };
      col += value.length;
      i = j;
    } else if (isdigit(c) || c === '.') {
      // 识别数字（非负整数或小数）
      let j = i;
      while (j < source.length && isdigit(source[j])) {
        j++;
      }
      if (source[j] === '.') {
        j++;
        while (j < source.length && isdigit(source[j])) {
          j++;
        }
      }
      let value = source.substring(i, j);
      yield { type: TokenType.number, value, line, col };
      col += value.length;
      i = j;
    } else if (c === '"') {
      // 识别字符串
      let j = i + 1;
      while (j < source.length && source[j] !== '"') {
        if (source[j] === '\\') {
          j++;
        }
        j++;
      }
      if (j >= source.length) {
        throw new Error(`Unterminated string literal at ${line}:${col}`);
      }
      let raw_value = source.substring(i + 1, j);
      let value = raw_value.replace(/\\"/g, '"');
      yield { type: TokenType.string, value, line, col };
      col += raw_value.length + 2;
      i = j + 1;
    } else if (c === '/') {
      // 识别正则表达式
      let j = i + 1;
      while (j < source.length && source[j] !== '/') {
        if (source[j] === '\\') {
          j++;
        }
        j++;
      }
      if (j >= source.length) {
        throw new Error(`Unterminated regexp literal at ${line}:${col}`);
      }
      let value = source.substring(i + 1, j);
      yield { type: TokenType.regexp, value, line, col };
      col += value.length + 2;
      i = j + 1;
    } else if ('()[]=,+'.indexOf(c) >= 0) {
      yield { type: TokenType.punctuator, value: c, line, col };
      col++;
      i++;
    } else {
      throw new Error(`Unexpected character ${c} at ${line}:${col}`);
    }
  }
  yield { type: TokenType.end, value: '', line, col };
}

function isspace(c: string) {
  return c === ' ' || c === '\t' || c === '\n' || c === '\r';
}

function isdigit(c: string) {
  return c >= '0' && c <= '9';
}

function isalpha(c: string) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

function isalnum(c: string) {
  return isalpha(c) || isdigit(c);
}
