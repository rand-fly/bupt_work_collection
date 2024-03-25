import { parse } from '../src/parse';

test('parse - single state with enter event', () => {
  const source = `state main
    enter
        say "Hello, world!"
`;
  const script = parse(source);
  expect(script.states).toEqual({
    main: {
      name: 'main',
      enterEvent: [{ type: 'say', text: { type: 'str', value: 'Hello, world!' } }],
      caseEvents: [],
      silentEvents: []
    }
  });
});

test('parse - single state with case event', () => {
  const source = `state main
    case "greeting"
        say "Hello!"
`;
  const script = parse(source);
  expect(script.states).toEqual({
    main: {
      name: 'main',
      caseEvents: [
        { cond: 'greeting', event: [{ type: 'say', text: { type: 'str', value: 'Hello!' } }] }
      ],
      silentEvents: []
    }
  });
});

test('parse - single state with default event', () => {
  const source = `state main
    default
        say "Unknown command"
`;
  const script = parse(source);
  expect(script.states).toEqual({
    main: {
      name: 'main',
      caseEvents: [],
      defaultEvent: [{ type: 'say', text: { type: 'str', value: 'Unknown command' } }],
      silentEvents: []
    }
  });
});

test('parse - single state with silent event', () => {
  const source = `state main
    silent 5
        say "Are you still there?"
`;
  const script = parse(source);
  expect(script.states).toEqual({
    main: {
      name: 'main',
      caseEvents: [],
      silentEvents: [
        { delay: 5, event: [{ type: 'say', text: { type: 'str', value: 'Are you still there?' } }] }
      ]
    }
  });
});

test('parse - complex script', () => {
  const source = `state main
    enter
        say "Welcome to ChattyCat!"
    case "greeting"
        say "Hello!"
    case /bye/
        say "Goodbye!"
    case "switch"
        goto state2
    default
        say "Unknown command"
    silent 10
        say "Are you still there?"
    silent 20
        say "I'm going to sleep now"

state state2
    enter
        say "Welcome to state2!"
    case "test"
        let $cond = "1"
        [$cond] say "the complex function returns " + function(a($var1) + b($var2), c("qwq" + $var3))
        [not($cond)] suggest "a", "b", "c"
        ["1"] delay 3.14159
    case "switch"
        goto main
`;
  const script = parse(source);
  expect(script.states).toEqual({
    main: {
      name: 'main',
      enterEvent: [{ type: 'say', text: { type: 'str', value: 'Welcome to ChattyCat!' } }],
      caseEvents: [
        { cond: 'greeting', event: [{ type: 'say', text: { type: 'str', value: 'Hello!' } }] },
        { cond: /bye/, event: [{ type: 'say', text: { type: 'str', value: 'Goodbye!' } }] },
        { cond: 'switch', event: [{ type: 'goto', stateName: 'state2' }] }
      ],
      defaultEvent: [{ type: 'say', text: { type: 'str', value: 'Unknown command' } }],
      silentEvents: [
        {
          delay: 10,
          event: [{ type: 'say', text: { type: 'str', value: 'Are you still there?' } }]
        },
        {
          delay: 20,
          event: [{ type: 'say', text: { type: 'str', value: "I'm going to sleep now" } }]
        }
      ]
    },
    state2: {
      name: 'state2',
      enterEvent: [{ type: 'say', text: { type: 'str', value: 'Welcome to state2!' } }],
      caseEvents: [
        {
          cond: 'test',
          event: [
            {
              type: 'let',
              lhs: '$cond',
              rhs: { type: 'str', value: '1' }
            },
            {
              type: 'say',
              text: {
                type: 'concat',
                lhs: {
                  type: 'str',
                  value: 'the complex function returns '
                },
                rhs: {
                  type: 'func',
                  name: 'function',
                  args: [
                    {
                      type: 'concat',
                      lhs: {
                        type: 'func',
                        name: 'a',
                        args: [{ type: 'var', value: '$var1' }]
                      },
                      rhs: {
                        type: 'func',
                        name: 'b',
                        args: [{ type: 'var', value: '$var2' }]
                      }
                    },
                    {
                      type: 'func',
                      name: 'c',
                      args: [
                        {
                          type: 'concat',
                          lhs: { type: 'str', value: 'qwq' },
                          rhs: { type: 'var', value: '$var3' }
                        }
                      ]
                    }
                  ]
                }
              },
              cond: { type: 'var', value: '$cond' }
            },
            {
              type: 'suggest',
              suggestions: [
                { type: 'str', value: 'a' },
                { type: 'str', value: 'b' },
                { type: 'str', value: 'c' }
              ],
              cond: {
                type: 'func',
                name: 'not',
                args: [{ type: 'var', value: '$cond' }]
              }
            },
            {
              type: 'delay',
              timeout: 3.14159,
              cond: { type: 'str', value: '1' }
            }
          ]
        },
        {
          cond: 'switch',
          event: [{ type: 'goto', stateName: 'main' }]
        }
      ],
      silentEvents: []
    }
  });
});

// 不在状态内写语句
test('parse - write statement outside state', () => {
  const source = `state main
say "Hello, world!"
`;
  expect(() => parse(source)).toThrow();
});

// 状态名是变量名
test('parse - state name is variable', () => {
  const source = `state $state
enter
    say "Hello, world!"
`;
  expect(() => parse(source)).toThrow();
});

// 调用一个变量名
test('parse - call a variable', () => {
  const source = `state main
enter
    say $var()
`;
  expect(() => parse(source)).toThrow();
});

// 在exit语句后写表达式
test('parse - write expression after exit', () => {
  const source = `state main
enter 
    exit 12
    say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// 在goto语句后写表达式
test('parse - write expression after goto', () => {
  const source = `state main
enter
    goto $var
    say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// 在delay语句后写表达式
test('parse - write expression after delay', () => {
  const source = `state main
enter
    delay $var
    say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// let语句的左值不是变量
test('parse - let statement with non-variable lhs', () => {
  const source = `state main
enter
    let main = "abc"
    say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// let语句缺少 =
test('parse - let statement without =', () => {
  const source = `state main
enter
    let $var "abc"
    say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// + 号前没有表达式
test('parse - + without lhs', () => {
  const source = `state main
enter
    say + "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// + 号后没有表达式
test('parse - + without rhs', () => {
  const source = `state main
enter
    say "ok?" +
`;
  expect(() => parse(source)).toThrow();
});

// 连续两个 + 号
test('parse - two + in a row', () => {
  const source = `state main
enter
    say "a" + + "b"
`;
  expect(() => parse(source)).toThrow();
});

// 缺少 + 号
test('parse - missing +', () => {
  const source = `state main
enter
    say "a" "b"
`;
  expect(() => parse(source)).toThrow();
});

// 函数调用的多余逗号
test('parse - extra comma in function call', () => {
  const source = `state main
enter
    say func(1, 2,)
`;
  expect(() => parse(source)).toThrow();
});

// 重复条件表达式
test('parse - duplicate condition', () => {
  const source = `state main
enter
    [$cond1] [$cond2] say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// 条件表达式缺少 ]
test('parse - missing ] in condition', () => {
  const source = `state main
enter
    [$cond say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// 重复定义state
test('parse - duplicate state', () => {
  const source = `state main
enter
    say "ok?"
state main
enter
    say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// 使用未定义的state
test('parse - use undefined state', () => {
  const source = `state main
enter
    goto state2
`;
  expect(() => parse(source)).toThrow();
});

// 没有定义main state
test('parse - no main state', () => {
  const source = `state state2
enter
    say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// 重复定义enter事件
test('parse - duplicate enter event', () => {
  const source = `state main
enter
    say "ok?"
enter
    say "ok?"
`;
  expect(() => parse(source)).toThrow();
});

// 重复定义default事件
test('parse - duplicate default event', () => {
  const source = `state main
default
    say "ok?"
default
    say "ok?"
`;
  expect(() => parse(source)).toThrow();
});
