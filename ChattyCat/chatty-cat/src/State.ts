import { Statements } from './Statement';

/**
 * 表示一个脚本中的状态
 */
export interface State {
  name: string;
  enterEvent?: Statements;
  caseEvents: { cond: string | RegExp; event: Statements }[];
  defaultEvent?: Statements;
  silentEvents: { delay: number; event: Statements }[];
}
