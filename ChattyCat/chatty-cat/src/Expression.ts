export type Expression =
  | {
      type: 'str';
      value: string;
    }
  | {
      type: 'var';
      value: string;
    }
  | {
      type: 'concat';
      lhs: Expression;
      rhs: Expression;
    }
  | {
      type: 'func';
      name: string;
      args: Expression[];
    };
