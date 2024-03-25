import { tokenize, TokenType } from '../src/tokenize';

test('tokenize - empty source', () => {
  const source = '';
  const tokens = Array.from(tokenize(source));
  expect(tokens).toEqual([{ type: TokenType.end, value: '', line: 1, col: 1 }]);
});

test('tokenize - whitespace', () => {
  const source = '   \n  \t  ';
  const tokens = Array.from(tokenize(source));
  expect(tokens).toEqual([{ type: TokenType.end, value: '', line: 2, col: 6 }]);
});

test('tokenize - comments', () => {
  const source = `# This is a comment
# Another comment
`;
  const tokens = Array.from(tokenize(source));
  expect(tokens).toEqual([{ type: TokenType.end, value: '', line: 3, col: 1 }]);
});

test('tokenize - words', () => {
  const source = 'hello world $variable _underscore';
  const tokens = Array.from(tokenize(source));
  expect(tokens).toEqual([
    { type: TokenType.word, value: 'hello', line: 1, col: 1 },
    { type: TokenType.word, value: 'world', line: 1, col: 7 },
    { type: TokenType.word, value: '$variable', line: 1, col: 13 },
    { type: TokenType.word, value: '_underscore', line: 1, col: 23 },
    { type: TokenType.end, value: '', line: 1, col: 34 }
  ]);
});

test('tokenize - numbers', () => {
  const source = '123 3.14 .0123 007.';
  const tokens = Array.from(tokenize(source));
  expect(tokens).toEqual([
    { type: TokenType.number, value: '123', line: 1, col: 1 },
    { type: TokenType.number, value: '3.14', line: 1, col: 5 },
    { type: TokenType.number, value: '.0123', line: 1, col: 10 },
    { type: TokenType.number, value: '007.', line: 1, col: 16 },
    { type: TokenType.end, value: '', line: 1, col: 20 }
  ]);
});

test('tokenize - strings', () => {
  const source = '"hello" "world" "\\""';
  const tokens = Array.from(tokenize(source));
  expect(tokens).toEqual([
    { type: TokenType.string, value: 'hello', line: 1, col: 1 },
    { type: TokenType.string, value: 'world', line: 1, col: 9 },
    { type: TokenType.string, value: '"', line: 1, col: 17 },
    { type: TokenType.end, value: '', line: 1, col: 21 }
  ]);
});

test('tokenize - regular expressions', () => {
  const source = '/[0-9]+/ /[a-z]+/ /[\\/\\\\]/';
  const tokens = Array.from(tokenize(source));
  expect(tokens).toEqual([
    { type: TokenType.regexp, value: '[0-9]+', line: 1, col: 1 },
    { type: TokenType.regexp, value: '[a-z]+', line: 1, col: 10 },
    { type: TokenType.regexp, value: '[\\/\\\\]', line: 1, col: 19 },
    { type: TokenType.end, value: '', line: 1, col: 27 }
  ]);
});

test('tokenize - punctuators', () => {
  const source = '()[]=,+';
  const tokens = Array.from(tokenize(source));
  expect(tokens).toEqual([
    { type: TokenType.punctuator, value: '(', line: 1, col: 1 },
    { type: TokenType.punctuator, value: ')', line: 1, col: 2 },
    { type: TokenType.punctuator, value: '[', line: 1, col: 3 },
    { type: TokenType.punctuator, value: ']', line: 1, col: 4 },
    { type: TokenType.punctuator, value: '=', line: 1, col: 5 },
    { type: TokenType.punctuator, value: ',', line: 1, col: 6 },
    { type: TokenType.punctuator, value: '+', line: 1, col: 7 },
    { type: TokenType.end, value: '', line: 1, col: 8 }
  ]);
});

test('tokenize - invalid character', () => {
  const source = 'hello @world';
  expect(() => Array.from(tokenize(source))).toThrow();
});

test('tokenize - unclosed string', () => {
  const source = '"hello world';
  expect(() => Array.from(tokenize(source))).toThrow();
});

test('tokenize - unclosed regular expression', () => {
  const source = '/[0-9]+';
  expect(() => Array.from(tokenize(source))).toThrow();
});

// Additional tests for your code
test('tokenize - complex source', () => {
  const source = `state main
    enter
        say ""
    case "你好"
        say "我确实很好"
    case /为什么/
        say "我不知道"
    default
        say random("对不起，我不能理解", "听不懂") + "哦"
    silent 10
        say "您好，请问您还在吗？"
`;
  const tokens = Array.from(tokenize(source));
  expect(tokens).toEqual([
    { type: TokenType.word, value: 'state', line: 1, col: 1 },
    { type: TokenType.word, value: 'main', line: 1, col: 7 },
    { type: TokenType.word, value: 'enter', line: 2, col: 5 },
    { type: TokenType.word, value: 'say', line: 3, col: 9 },
    { type: TokenType.string, value: '', line: 3, col: 13 },
    { type: TokenType.word, value: 'case', line: 4, col: 5 },
    { type: TokenType.string, value: '你好', line: 4, col: 10 },
    { type: TokenType.word, value: 'say', line: 5, col: 9 },
    { type: TokenType.string, value: '我确实很好', line: 5, col: 13 },
    { type: TokenType.word, value: 'case', line: 6, col: 5 },
    { type: TokenType.regexp, value: '为什么', line: 6, col: 10 },
    { type: TokenType.word, value: 'say', line: 7, col: 9 },
    { type: TokenType.string, value: '我不知道', line: 7, col: 13 },
    { type: TokenType.word, value: 'default', line: 8, col: 5 },
    { type: TokenType.word, value: 'say', line: 9, col: 9 },
    { type: TokenType.word, value: 'random', line: 9, col: 13 },
    { type: TokenType.punctuator, value: '(', line: 9, col: 19 },
    { type: TokenType.string, value: '对不起，我不能理解', line: 9, col: 20 },
    { type: TokenType.punctuator, value: ',', line: 9, col: 31 },
    { type: TokenType.string, value: '听不懂', line: 9, col: 33 },
    { type: TokenType.punctuator, value: ')', line: 9, col: 38 },
    { type: TokenType.punctuator, value: '+', line: 9, col: 40 },
    { type: TokenType.string, value: '哦', line: 9, col: 42 },
    { type: TokenType.word, value: 'silent', line: 10, col: 5 },
    { type: TokenType.number, value: '10', line: 10, col: 12 },
    { type: TokenType.word, value: 'say', line: 11, col: 9 },
    { type: TokenType.string, value: '您好，请问您还在吗？', line: 11, col: 13 },
    { type: TokenType.end, value: '', line: 12, col: 1 }
  ]);
});
