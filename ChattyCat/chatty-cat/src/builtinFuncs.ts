export const builtinFuncs: Record<string, (...args: string[]) => string> = {
  iff(cond, t, f) {
    if (cond === undefined || t === undefined || f === undefined)
      throw new Error('iff: too few arguments');
    return cond !== '0' ? t : f;
  },
  and(...args) {
    return args.every((x) => x !== '0') ? '1' : '0';
  },
  or(...args) {
    return args.some((x) => x !== '0') ? '1' : '0';
  },
  not(a) {
    if (a === undefined) throw new Error('not: too few arguments');
    return a === '0' ? '1' : '0';
  },
  eq(a, b) {
    if (a === undefined || b === undefined) throw new Error('eq: too few arguments');
    return a === b ? '1' : '0';
  },
  neq(a, b) {
    if (a === undefined || b === undefined) throw new Error('neq: too few arguments');
    return a !== b ? '1' : '0';
  },
  len(a) {
    if (a === undefined) throw new Error('len: too few arguments');
    return a.length.toString();
  },
  strcmp(a, b) {
    if (a === undefined || b === undefined) throw new Error('strcmp: too few arguments');
    return a === b ? '0' : a > b ? '1' : '-1';
  },
  numcmp(a, b) {
    if (a === undefined || b === undefined) throw new Error('numcmp: too few arguments');
    const numa = Number(a);
    if (isNaN(numa)) throw new Error(`numcmp: ${a} is not a number`);
    const numb = Number(b);
    if (isNaN(numb)) throw new Error(`numcmp: ${b} is not a number`);
    return numa === numb ? '0' : numa > numb ? '1' : '-1';
  },
  add(a, b) {
    if (a === undefined || b === undefined) throw new Error('add: too few arguments');
    const numa = Number(a);
    if (isNaN(numa)) throw new Error(`add: ${a} is not a number`);
    const numb = Number(b);
    if (isNaN(numb)) throw new Error(`add: ${b} is not a number`);
    return (numa + numb).toString();
  },
  sub(a, b) {
    if (a === undefined || b === undefined) throw new Error('sub: too few arguments');
    const numa = Number(a);
    if (isNaN(numa)) throw new Error(`sub: ${a} is not a number`);
    const numb = Number(b);
    if (isNaN(numb)) throw new Error(`sub: ${b} is not a number`);
    return (numa - numb).toString();
  },
  mul(a, b) {
    if (a === undefined || b === undefined) throw new Error('mul: too few arguments');
    const numa = Number(a);
    if (isNaN(numa)) throw new Error(`mul: ${a} is not a number`);
    const numb = Number(b);
    if (isNaN(numb)) throw new Error(`mul: ${b} is not a number`);
    return (numa * numb).toString();
  },
  div(a, b) {
    if (a === undefined || b === undefined) throw new Error('div: too few arguments');
    const numa = Number(a);
    if (isNaN(numa)) throw new Error(`div: ${a} is not a number`);
    const numb = Number(b);
    if (isNaN(numb)) throw new Error(`div: ${b} is not a number`);
    return (numa / numb).toString();
  },
  mod(a, b) {
    if (a === undefined || b === undefined) throw new Error('mod: too few arguments');
    const numa = Number(a);
    if (isNaN(numa)) throw new Error(`mod: ${a} is not a number`);
    const numb = Number(b);
    if (isNaN(numb)) throw new Error(`mod: ${b} is not a number`);
    return (numa % numb).toString();
  },
  random(...args) {
    if (args.length === 0) throw new Error('random: too few arguments');
    return args[Math.floor(Math.random() * args.length)];
  },
  randomInt(min, max) {
    if (min === undefined || max === undefined) throw new Error('randomInt: too few arguments');
    const numMin = Number(min);
    const numMax = Number(max);
    if (isNaN(numMin) || Math.floor(numMin) !== numMin) {
      throw new Error(`randomInt: ${min} is not an integer`);
    }
    if (isNaN(numMax) || Math.floor(numMax) !== numMax) {
      throw new Error(`randomInt: ${max} is not an integer`);
    }
    if (numMin > numMax) {
      throw new Error(`randomInt: min is greater than max`);
    }
    return Math.floor(Math.random() * (numMax - numMin + 1) + numMin).toString();
  },
  time() {
    return new Date().toLocaleTimeString();
  },
  date() {
    return new Date().toLocaleDateString();
  }
};
