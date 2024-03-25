import { State } from './State';

/**
 * 脚本对象
 */
export interface Script {
  states: Record<string, State>;
}
