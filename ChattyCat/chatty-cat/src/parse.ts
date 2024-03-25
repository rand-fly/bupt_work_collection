import { Token, TokenType, tokenize } from './tokenize';
import { Script } from './Script';
import { State } from './State';
import { Statement, Statements } from './Statement';
import { Expression } from './Expression';

/**
 * 解析给定的源代码字符串，返回一个脚本对象。
 * @param source 源代码字符串
 * @returns 解析后的脚本对象
 */
export function parse(source: string): Script {
  let tokens = tokenize(source);
  let next = tokens.next().value as Token;

  // 记录所有 goto 后的 state 名字，用于在最后检查是否存在未定义的 state
  let stateNamesToCheck = [] as Token[];

  let script: Script = { states: {} };
  for (;;) {
    if (tryType(TokenType.end)) break;
    let state = parseState();
    script.states[state.name] = state;
  }
  for (let token of stateNamesToCheck) {
    if (script.states[token.value] === undefined) {
      throw new Error(`state ${token.value} is not defined at line ${token.line} col ${token.col}`);
    }
  }
  if (script.states['main'] === undefined) {
    throw new Error(`state main is not defined`);
  }
  return script;

  // 向后读取一个 token
  function forward() {
    let token = tokens.next();
    if (!token.done) next = token.value;
  }
  // 尝试读取一个 token，如果为指定类型则返回该 token 并向后读取，否则返回 null
  function tryType(type: TokenType) {
    if (next.type === type) {
      let val = next;
      forward();
      return val;
    }
    return null;
  }
  // 尝试读取一个 token，如果为指定关键字则返回该 token 并向后读取，否则返回 null
  function tryKeyword(keyword: string) {
    if (next.type === TokenType.word && next.value === keyword) {
      forward();
      return true;
    }
    return false;
  }
  // 尝试读取一个 token，如果为指定符号则返回该 token 并向后读取，否则返回 null
  function tryPunctuator(punctuator: string) {
    if (next.type === TokenType.punctuator && next.value === punctuator) {
      forward();
      return true;
    }
    return false;
  }
  // 期望读取一个指定类型的 token，如果成功则返回该 token 向后读取，否则抛出异常
  function expectType(type: TokenType) {
    let res = tryType(type);
    if (res) return res;
    throw new Error(`expect ${TokenType[type]} at line ${next.line} col ${next.col}`);
  }
  // 期望读取一个指定关键字的 token，如果成功则返回该 token 向后读取，否则抛出异常
  function expectKeyword(keyword: string) {
    if (tryKeyword(keyword)) return true;
    throw new Error(`expect ${keyword} at line ${next.line} col ${next.col}`);
  }
  // 期望读取一个指定符号的 token，如果成功则返回该 token 向后读取，否则抛出异常
  function expectPunctuator(punctuator: string) {
    if (tryPunctuator(punctuator)) return true;
    throw new Error(`expect ${punctuator} at line ${next.line} col ${next.col}`);
  }
  function isVarName(name: string) {
    return name[0] === '$';
  }

  function parseState() {
    expectKeyword('state');
    let name = expectType(TokenType.word).value;
    if (isVarName(name)) {
      throw new Error(`state name can not be variable at line ${next.line} col ${next.col}`);
    }
    if (script.states[name] !== undefined) {
      throw new Error(`duplicate state name ${name} at line ${next.line} col ${next.col}`);
    }
    let state: State = { name, caseEvents: [], silentEvents: [] };
    for (;;) {
      if (tryKeyword('enter')) {
        if (state.enterEvent) {
          throw new Error(`duplicate enter event at line ${next.line} col ${next.col}`);
        }
        state.enterEvent = parseStatements();
      } else if (tryKeyword('case')) {
        let tmp: Token | null;
        if ((tmp = tryType(TokenType.string))) {
          state.caseEvents.push({ cond: tmp.value, event: parseStatements() });
        } else if ((tmp = tryType(TokenType.regexp))) {
          state.caseEvents.push({ cond: RegExp(tmp.value), event: parseStatements() });
        }
      } else if (tryKeyword('default')) {
        if (state.defaultEvent) {
          throw new Error(`duplicate default event at line ${next.line} col ${next.col}`);
        }
        state.defaultEvent = parseStatements();
      } else if (tryKeyword('silent')) {
        let delay = expectType(TokenType.number).value;
        state.silentEvents.push({ delay: Number(delay), event: parseStatements() });
      } else {
        break;
      }
    }
    return state;
  }

  function parseStatements() {
    let statements: Statements = [];
    for (;;) {
      let cond: Expression | null = null;
      if (tryPunctuator('[')) {
        cond = parseExpression();
        expectPunctuator(']');
      }
      let statement: Statement;
      if (tryKeyword('say')) {
        let text = parseExpression();
        statement = { type: 'say', text };
      } else if (tryKeyword('suggest')) {
        let suggestions = [parseExpression()];
        while (tryPunctuator(',')) {
          suggestions.push(parseExpression());
        }
        statement = { type: 'suggest', suggestions };
      } else if (tryKeyword('delay')) {
        let timeout = expectType(TokenType.number).value;
        statement = { type: 'delay', timeout: Number(timeout) };
      } else if (tryKeyword('goto')) {
        let stateNameToken = next;
        let stateName = expectType(TokenType.word).value;
        if (isVarName(stateName)) {
          throw new Error(`state name can not be variable at line ${next.line} col ${next.col}`);
        }
        stateNamesToCheck.push(stateNameToken);
        statement = { type: 'goto', stateName };
      } else if (tryKeyword('exit')) {
        statement = { type: 'exit' };
      } else if (tryKeyword('let')) {
        let lhs = expectType(TokenType.word).value;
        if (!isVarName(lhs)) {
          throw new Error(`variable name must start with $ at line ${next.line} col ${next.col}`);
        }
        expectPunctuator('=');
        let rhs = parseExpression();
        statement = { type: 'let', lhs, rhs };
      } else {
        break;
      }
      if (cond !== null) {
        statement.cond = cond;
      }
      statements.push(statement);
    }
    return statements;
  }

  function parseExpression(): Expression {
    let lhs = parseTerm();
    for (;;) {
      if (tryPunctuator('+')) {
        let rhs = parseTerm();
        lhs = { type: 'concat', lhs, rhs };
      } else {
        break;
      }
    }
    return lhs;
  }

  function parseTerm(): Expression {
    let tmp;
    if ((tmp = tryType(TokenType.string))) {
      return { type: 'str', value: tmp.value };
    } else if ((tmp = tryType(TokenType.word))) {
      if (isVarName(tmp.value)) {
        return { type: 'var', value: tmp.value };
      } else {
        const args: Expression[] = [];
        expectPunctuator('(');
        if (!tryPunctuator(')')) {
          for (;;) {
            args.push(parseExpression());
            if (!tryPunctuator(',')) {
              expectPunctuator(')');
              break;
            }
          }
        }
        return { type: 'func', name: tmp.value, args };
      }
    } else {
      throw new Error(`expect string or word at line ${next.line} col ${next.col}`);
    }
  }
}
