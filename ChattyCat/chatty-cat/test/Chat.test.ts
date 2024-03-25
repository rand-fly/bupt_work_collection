import { Chat } from '../src/Chat';
import { Script } from '../src/Script';
import { State } from '../src/State';
import { parse } from '../src/parse';

// 便于测试的工具
export function testChat(
  script: Script,
  variables?: { [name: string]: string },
  funcs?: { [name: string]: (...args: string[]) => string | Promise<string> }
) {
  const outputQueue: string[] = [];
  const suggestQueue: string[][] = [];
  const errorQueue: string[] = [];
  return {
    chat: new Chat({
      script,
      variables,
      funcs,
      output: (text) => outputQueue.push(text),
      suggest: (texts) => suggestQueue.push(texts),
      error: (error) => errorQueue.push(error)
    }),
    expectOutput(text: string) {
      expect(outputQueue.length).toBeGreaterThan(0);
      expect(outputQueue.shift()).toEqual(text);
    },
    expectSuggest(texts: string[]) {
      expect(suggestQueue.length).toBeGreaterThan(0);
      expect(suggestQueue.shift()).toEqual(texts);
    },
    expectError() {
      expect(errorQueue.length).toBeGreaterThan(0);
      expect(errorQueue.shift());
    },
    expectNoMore() {
      expect(outputQueue.length).toEqual(0);
      expect(suggestQueue.length).toEqual(0);
      expect(errorQueue.length).toEqual(0);
    }
  };
}

// 使用jest提供的fake timer作为测试桩便于测试silent和delay
jest.useFakeTimers();

test('say and suggest', async () => {
  const script = parse(
    `state main
    enter
        say "hello"
        say "world"
        say "blablabla"
        say "blablabla"
        suggest "a", "b", "c", "d", "e", "f", "g"
        suggest "h", "i", "j", "k", "l", "m", "n"
`
  );
  const { chat, expectOutput, expectSuggest, expectNoMore } = testChat(script);
  await chat.start();
  expectOutput('hello');
  expectOutput('world');
  expectOutput('blablabla');
  expectOutput('blablabla');
  expectSuggest(['a', 'b', 'c', 'd', 'e', 'f', 'g']);
  expectSuggest(['h', 'i', 'j', 'k', 'l', 'm', 'n']);
  expectNoMore();
});

test('string and regexp match', async () => {
  const script = parse(
    `state main
    enter
        say "hello"
    case "abc"
        say "match 1"
    case "abc"
        say "match 2"
    case /abc/
        say "match 3 " + $0
    case /(.*)bc/
        say "match 4 " + $0 + " " + $1
    case /.*/
        say "match 5 " + $0
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  expectOutput('hello');
  await chat.input('abc');
  expectOutput('match 1');
  await chat.input('abcdefg');
  expectOutput('match 3 abc');
  await chat.input('bbc');
  expectOutput('match 4 bbc b');
  await chat.input('zzz');
  expectOutput('match 5 zzz');
  expectNoMore();
});

test('state transition', async () => {
  const script = parse(
    `state next
    enter
        say "world"
    default
        goto main
state main
    enter
        say "hello"
        goto next

`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  expectOutput('hello');
  expectOutput('world');
  await chat.input('hey');
  expectOutput('hello');
  expectOutput('world');
  expectNoMore();
});

test('state transition and silent', async () => {
  const script = parse(
    `state main
    silent 1
        say "how"
        goto state2
    silent 2
        say "blabla" # this should not be output

state state2
    enter
        say "are"
    silent 2
        say "you"
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  await jest.advanceTimersByTimeAsync(1000);
  expectOutput('how');
  expectOutput('are');
  await jest.advanceTimersByTimeAsync(2000);
  expectOutput('you');
  expectNoMore();
});

test('delay suspending', async () => {
  const script = parse(
    `state main
    enter
        say "nice"
    silent 0
        say "to"
        delay 1.5
    silent 1
        say "blabla" # this should not be output
    silent 2
        say "meet"
    silent 2
        say "you"
    default
        say "hey" # this should not be output
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  expectOutput('nice');
  await jest.advanceTimersByTimeAsync(0);
  expectOutput('to');
  await jest.advanceTimersByTimeAsync(500);
  await chat.input('hey');
  await jest.advanceTimersByTimeAsync(2000);
  expectOutput('meet');
  expectOutput('you');
  expectNoMore();
});

test('start twice', async () => {
  const script = parse(
    `state main
    enter
        say "hey"
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  await chat.start();
  expectOutput('hey');
  expectNoMore();
});

test('input after stop', async () => {
  const script = parse(
    `state main
    default
        say "hey"
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  await chat.input('hello');
  expectOutput('hey');
  chat.stop();
  await chat.input('hello');
  expectNoMore();
});

test('stop twice', async () => {
  const script = parse(
    `state main
    default
        say "hey"
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  await chat.input('hello');
  expectOutput('hey');
  chat.stop();
  chat.stop();
  await chat.input('hello');
  expectNoMore();
});

test('stop when suspending(delay)', async () => {
  const script = parse(
    `state main
    enter
        say "hello"
        delay 2
        say "blabla"
        suggest "a", "b"
        let $a = "1"
        let $a = $b # try to make an error
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  chat.start();
  await jest.advanceTimersByTimeAsync(0);
  expectOutput('hello');
  await jest.advanceTimersByTimeAsync(1000);
  chat.stop();
  await jest.advanceTimersByTimeAsync(2000);
  expectNoMore();
});

test('stop when suspending(async function)', async () => {
  const script = parse(
    `state main
    enter
        say "hello"
        say get() + get()
        say "blabla"
        suggest "a", "b"
        let $a = "1"
        let $a = $b # try to make an error
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(
    script,
    {},
    {
      async get() {
        await new Promise((resolve) => setTimeout(resolve, 2000));
        return 'bla';
      }
    }
  );
  chat.start();
  await jest.advanceTimersByTimeAsync(0);
  expectOutput('hello');
  await jest.advanceTimersByTimeAsync(1000);
  chat.stop();
  await jest.advanceTimersByTimeAsync(2000);
  expectNoMore();
});

test('exit', async () => {
  const script = parse(
    `state main
    default
        say "hello"
        exit
    silent 1
        say "blabla" # this should not be output
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  await chat.input('hello');
  expectOutput('hello');
  await chat.input('hello');
  await jest.advanceTimersByTimeAsync(1000);
  expectNoMore();
});

test('concat variables', async () => {
  const script = parse(
    `state main
    enter
        let $a = "hello"
        let $b = "world"
        let $c = $a + " " + $b
        say $c
        let $d = "nice to meet you"
        say $d
        let $e = $c + " " + $d
        say $e
        let $f = $c + " " + $d + " " + $e
        say $f
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  expectOutput('hello world');
  expectOutput('nice to meet you');
  expectOutput('hello world nice to meet you');
  expectOutput('hello world nice to meet you hello world nice to meet you');
  expectNoMore();
});

test('conditional', async () => {
  const script = parse(
    `state main
    enter
        let $a = ""
        let $b = "a"
        let $c = "0"
        [$a] say "hello"
        [$b] say "world"
        [$c] say "blabla" # this should not be output
        ["0" + "0"] suggest "a"
`
  );
  const { chat, expectOutput, expectSuggest, expectNoMore } = testChat(script);
  await chat.start();
  expectOutput('hello');
  expectOutput('world');
  expectSuggest(['a']);
  expectNoMore();
});

test('builtin functions', async () => {
  const script = parse(
    `state main
    case /\\d+/
        let $ans = "1"
        let $i = $0
        goto fac
    
state fac
    enter
        [$i] let $ans = mul($ans, $i)
        [$i] let $i = sub($i, "1")
        [$i] goto fac
        say $ans
        goto main
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script);
  await chat.start();
  await chat.input('5');
  expectOutput('120');
  await chat.input('10');
  expectOutput('3628800');
  expectNoMore();
});

test('predefined variable', async () => {
  const script = parse(
    `state main
    enter
        say "my name is " + $name
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(script, { $name: 'Alice' });
  await chat.start();
  expectOutput('my name is Alice');
  expectNoMore();
});

test('custom function', async () => {
  const script = parse(
    `state main
    enter
        say "my name is " + getAttr("name")
`
  );
  const { chat, expectOutput, expectNoMore } = testChat(
    script,
    {},
    {
      getAttr(attr) {
        if (attr === 'name') return 'Alice';
        else throw new Error('unknown attr ' + attr);
      }
    }
  );
  await chat.start();
  expectOutput('my name is Alice');
  expectNoMore();
});

test('custom async function', async () => {
  const script = parse(
    `state main
    enter
        say "my name is " + getAttr("name")
`
  );
  let resolver: (value: string) => void;
  const { chat, expectOutput, expectNoMore } = testChat(
    script,
    {},
    {
      async getAttr(attr) {
        if (attr === 'name')
          return new Promise<string>((resolve) => {
            resolver = resolve;
          });
        else throw new Error('unknown attr ' + attr);
      }
    }
  );
  chat.start();
  await jest.advanceTimersByTimeAsync(0);
  resolver!('Alice');
  await jest.advanceTimersByTimeAsync(0);
  expectOutput('my name is Alice');
  expectNoMore();
});

test('function not found', async () => {
  const script = parse(
    `state main
    enter
        say "my name is " + name()
`
  );
  const { chat, expectError, expectNoMore } = testChat(script);
  await chat.start();
  expectError();
  expectNoMore();
});

test('function return non-string', async () => {
  const script = parse(
    `state main
    enter
        say "my name is " + getAttr("name")
`
  );
  const { chat, expectError, expectNoMore } = testChat(script, {}, {
    getAttr(attr: string) {
      if (attr === 'name') return 123;
      else throw new Error('unknown attr ' + attr);
    }
  } as any);
  await chat.start();
  expectError();
  expectNoMore();
});

test('async function return non-string', async () => {
  const script = parse(
    `state main
    enter
        say "my name is " + getAttr("name")
`
  );
  let resolver: (value: string) => void;
  const { chat, expectError, expectNoMore } = testChat(
    script,
    {},
    {
      async getAttr(attr) {
        if (attr === 'name')
          return new Promise<string>((resolve) => {
            resolver = resolve;
          });
        else throw new Error('unknown attr ' + attr);
      }
    }
  );
  chat.start();
  await jest.advanceTimersByTimeAsync(0);
  resolver!(123 as any);
  await jest.advanceTimersByTimeAsync(0);
  expectError();
  expectNoMore();
});

test('function throw', async () => {
  const script = parse(
    `state main
    enter
        say "my name is " + getAttr("age")
`
  );
  const { chat, expectError, expectNoMore } = testChat(script, {}, {
    getAttr(attr: string) {
      if (attr === 'name') return 'Alice';
      else throw new Error('unknown attr ' + attr);
    }
  } as any);
  await chat.start();
  expectError();
  expectNoMore();
});

test('async function throw', async () => {
  const script = parse(
    `state main
    enter
        say "my name is " + getAttr("age")
`
  );
  let resolver: (value: string) => void;
  const { chat, expectError, expectNoMore } = testChat(
    script,
    {},
    {
      async getAttr(attr) {
        if (attr === 'name')
          return new Promise<string>((resolve) => {
            resolver = resolve;
          });
        else throw new Error('unknown attr ' + attr);
      }
    }
  );
  chat.start();
  await jest.advanceTimersByTimeAsync(0);
  expectError();
  expectNoMore();
});

test('no main state', async () => {
  expect(() => new Chat({ script: { states: {} }, output: () => {} })).toThrow();
});

test('goto invalid state', async () => {
  const mainState: State = {
    name: 'main',
    enterEvent: [
      { type: 'suggest', suggestions: [{ type: 'str', value: 'a' }] },
      { type: 'goto', stateName: 'state2' }
    ],
    caseEvents: [],
    silentEvents: []
  };
  let isError = false;
  const chat = new Chat({
    script: { states: { main: mainState } },
    output: () => {},
    error() {
      isError = true;
    }
  });
  await chat.start();
  expect(isError).toBeTruthy();
});
