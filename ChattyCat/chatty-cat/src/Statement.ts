import { Expression } from './Expression';

type StatementSay = { type: 'say'; text: Expression };
type StatementSuggest = { type: 'suggest'; suggestions: Expression[] };
type StatementDelay = { type: 'delay'; timeout: number };
type StatementGoto = { type: 'goto'; stateName: string };
type StatementExit = { type: 'exit' };
type StatementLet = { type: 'let'; lhs: string; rhs: Expression };
export type Statement = (
  | StatementSay
  | StatementSuggest
  | StatementDelay
  | StatementGoto
  | StatementExit
  | StatementLet
) & { cond?: Expression };
export type Statements = Statement[];
