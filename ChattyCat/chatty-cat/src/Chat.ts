import { Expression } from './Expression';
import { Script } from './Script';
import { State } from './State';
import { Statements } from './Statement';
import { builtinFuncs } from './builtinFuncs';

type TID = ReturnType<typeof setTimeout>;

/**
 * 创建对话实例时指定的选项
 */
export interface ChatOptions {
  /**
   * 用于创建实例的脚本
   */
  script: Script;
  /**
   * 可选的初始变量集合
   */
  variables?: Record<string, string>;
  /**
   * 可选的自定义外部函数集合
   */
  funcs?: Record<string, (...args: string[]) => string | Promise<string>>;
  /**
   * 输出回调函数
   * @param text 输出的文本
   */
  output: (text: string) => void;
  /**
   * 可选的建议回调函数
   * @param texts 包含一组建议的文本数组
   */
  suggest?: (texts: string[]) => void;
  /**
   * 可选的错误处理回调函数，在发生运行时错误时调用
   * @param error 错误信息
   */
  error?: (error: string) => void;
  /**
   * 可选的更新回调函数，在对话实例的内部状态发生变换时调用
   */
  update?: () => void;
}

/**
 * 对话实例
 *
 * 创建对话实例后需要首先调用 start 方法启动对话，然后可以调用 input 方法输入文本。
 *
 * 一个对话实例包含如下上下文信息：
 * - 当前状态
 * - 变量集合
 * - 外部函数集合
 * - 是否退出
 * - 是否处于挂起状态
 *
 * 当以上信息发生变化时，会调用 update 回调函数，以便更新 UI。也可以在使用过程中直接修改变量集合和外部函数集合。
 */
export class Chat {
  private script: Script;
  private output: (text: string) => void;
  private suggest: (texts: string[]) => void;
  private error: (error: string) => void;
  private update: () => void;
  private timeoutHandles: TID[] = [];

  // 外部可见的上下文状态
  /** 当前状态 */
  currentState: State;
  /** 变量集合 */
  variables: Record<string, string>;
  /** 外部函数集合 */
  funcs: Record<string, ((...args: string[]) => string | Promise<string>) | undefined>;
  /** 是否退出 */
  exited = true;
  /** 是否处于挂起状态 */
  suspending = false;

  /**
   * 创建对话实例。
   * @param options 对话实例的选项
   */
  constructor(options: ChatOptions) {
    this.script = options.script;
    this.variables = options.variables || {};
    this.funcs = { ...options.funcs, ...builtinFuncs }; // 内置函数会覆盖同名的自定义函数
    this.output = options.output;
    this.suggest = options.suggest || (() => {});
    this.error = options.error || console.error;
    this.update = options.update || (() => {});
    let mainState = this.script.states['main'];
    if (!mainState) throw new Error('no main state');
    this.currentState = mainState;
  }

  /**
   * 启动对话实例，通常这会触发main状态的enter事件，并开始main状态的silent事件计时。
   * 如果脚本已经启动，则不会有任何效果。
   * @returns 一个 Promise，当所有相关语句全部执行完毕时 resolve
   */
  async start() {
    if (!this.exited) return;
    this.exited = false;
    this.updateTimeout();
    if (this.currentState.enterEvent) {
      await this.exec(this.currentState.enterEvent);
    }
  }

  /**
   * 输入文本。
   * 如果脚本已经退出或者处于挂起状态，则不会有任何效果。
   * @param text 输入的文本
   * @returns 一个 Promise，当所有相关语句全部执行完毕时 resolve
   */
  async input(text: string) {
    if (this.exited || this.suspending) return;
    this.updateTimeout();
    let matched = false;
    // 选择第一个匹配输入的case的事件
    for (let event of this.currentState.caseEvents) {
      if (typeof event.cond === 'string') {
        if (text === event.cond) {
          await this.exec(event.event);
          matched = true;
          break;
        }
      } else {
        const result = text.match(event.cond);
        if (result !== null) {
          // 将正则表达式的捕获组存入变量
          for (let i = 0; i < result.length; i++) {
            this.variables[`$${i}`] = result[i];
          }
          this.update();
          await this.exec(event.event);
          matched = true;
          break;
        }
      }
    }
    // 如果没有匹配的case，则执行default事件
    if (!matched && this.currentState.defaultEvent) {
      await this.exec(this.currentState.defaultEvent);
    }
  }

  /**
   * 停止对话实例。
   * 如果脚本已经退出，则不会有任何效果。
   * 调用后会触发一次 update 回调函数通知 exited 变量发生变化，此后不会再触发任何回调函数。
   */
  stop() {
    if (this.exited) return;
    for (let handle of this.timeoutHandles) {
      clearTimeout(handle);
    }
    this.timeoutHandles = [];
    this.exited = true;
    this.output = () => {};
    this.suggest = () => {};
    this.error = () => {};
    let update = this.update;
    this.update = () => {};
    update();
  }

  // 更新silent事件计时器
  private updateTimeout() {
    for (let handle of this.timeoutHandles) {
      clearTimeout(handle);
    }
    this.timeoutHandles = [];
    for (let event of this.currentState.silentEvents) {
      let handle = setTimeout(() => {
        if (this.exited || this.suspending) return;
        this.exec(event.event);
      }, event.delay * 1000);
      this.timeoutHandles.push(handle);
    }
  }

  private async exec(statements: Statements) {
    try {
      forLoop: for (let statement of statements) {
        if (statement.cond && (await this.eval(statement.cond)) === '0') continue;
        switch (statement.type) {
          case 'say':
            this.output(await this.eval(statement.text));
            break;
          case 'suggest':
            this.suggest(await Promise.all(statement.suggestions.map((e) => this.eval(e))));
            break;
          case 'delay':
            if (!this.suspending) {
              this.suspending = true;
              this.update();
            }
            let timeout = statement.timeout;
            await new Promise((resolve) => setTimeout(resolve, timeout * 1000));
            break;
          case 'goto':
            let newState = this.script.states[statement.stateName];
            if (!newState) throw new Error(`state ${statement.stateName} not found`);
            this.currentState = newState;
            this.update();
            this.updateTimeout();
            if (newState.enterEvent) {
              await this.exec(newState.enterEvent);
            }
            break forLoop; // goto语句后面的语句不会再执行
          case 'exit':
            this.stop();
            break forLoop; // exit语句后面的语句不会再执行
          case 'let':
            this.variables[statement.lhs] = await this.eval(statement.rhs);
            this.update();
            break;
        }
      }
    } catch (e) {
      this.error((e as Error).message);
    }
    if (this.suspending) {
      this.suspending = false;
      this.update();
    }
  }

  private async eval(exp: Expression): Promise<string> {
    if (exp.type === 'str') {
      return exp.value;
    } else if (exp.type === 'var') {
      const val = this.variables[exp.value];
      if (val === undefined) throw new Error(`variable ${exp.value} not found`);
      return val;
    } else if (exp.type === 'concat') {
      return (await this.eval(exp.lhs)) + (await this.eval(exp.rhs));
    } else {
      const evaled_args = await Promise.all(exp.args.map((arg) => this.eval(arg)));
      const func = this.funcs[exp.name];
      if (!func) throw new Error(`function ${exp.name} not found`);
      const result = func(...evaled_args);
      if (result instanceof Promise) {
        if (!this.suspending) {
          this.suspending = true;
          this.update();
        }
        let awaitedResult = await result;
        if (typeof awaitedResult !== 'string') {
          throw new Error(`async function ${exp.name} returns non-string value`);
        }
        return awaitedResult;
      } else {
        if (typeof result !== 'string') {
          throw new Error(`function ${exp.name} returns non-string value`);
        }
        return result;
      }
    }
  }
}
