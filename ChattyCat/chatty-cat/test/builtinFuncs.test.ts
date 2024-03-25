import { builtinFuncs } from '../src/builtinFuncs';

test('builtinFuncs - iff', () => {
  expect(builtinFuncs.iff('1', 'true', 'false')).toBe('true');
  expect(builtinFuncs.iff('0', 'true', 'false')).toBe('false');
  expect(() => builtinFuncs.iff('1', 'true')).toThrow();
  expect(() => builtinFuncs.iff('1')).toThrow();
  expect(() => builtinFuncs.iff()).toThrow();
});

test('builtinFuncs - and', () => {
  expect(builtinFuncs.and()).toBe('1');
  expect(builtinFuncs.and('1', '1', '1')).toBe('1');
  expect(builtinFuncs.and('1', '0', '1')).toBe('0');
});

test('builtinFuncs - or', () => {
  expect(builtinFuncs.or()).toBe('0');
  expect(builtinFuncs.or('0', '0', '0')).toBe('0');
  expect(builtinFuncs.or('0', '1', '0')).toBe('1');
});

test('builtinFuncs - not', () => {
  expect(builtinFuncs.not('0')).toBe('1');
  expect(builtinFuncs.not('1')).toBe('0');
  expect(() => builtinFuncs.not()).toThrow();
});

test('builtinFuncs - eq', () => {
  expect(builtinFuncs.eq('1', '1')).toBe('1');
  expect(builtinFuncs.eq('1', '0')).toBe('0');
  expect(() => builtinFuncs.eq('1')).toThrow();
  expect(() => builtinFuncs.eq()).toThrow();
});

test('builtinFuncs - len', () => {
  expect(builtinFuncs.len('')).toBe('0');
  expect(builtinFuncs.len('abcdef')).toBe('6');
  expect(() => builtinFuncs.len()).toThrow();
});

test('builtinFuncs - neq', () => {
  expect(builtinFuncs.neq('1', '1')).toBe('0');
  expect(builtinFuncs.neq('1', '0')).toBe('1');
  expect(() => builtinFuncs.neq('1')).toThrow();
  expect(() => builtinFuncs.neq()).toThrow();
});

test('builtinFuncs - strcmp', () => {
  expect(builtinFuncs.strcmp('abc', 'abc')).toBe('0');
  expect(builtinFuncs.strcmp('abc', 'def')).toBe('-1');
  expect(builtinFuncs.strcmp('def', 'abc')).toBe('1');
  expect(() => builtinFuncs.strcmp('abc')).toThrow();
  expect(() => builtinFuncs.strcmp()).toThrow();
});

test('builtinFuncs - numcmp', () => {
  expect(builtinFuncs.numcmp('1', '1')).toBe('0');
  expect(builtinFuncs.numcmp('1', '2')).toBe('-1');
  expect(builtinFuncs.numcmp('2', '1')).toBe('1');
  expect(() => builtinFuncs.numcmp('abc', '2')).toThrow();
  expect(() => builtinFuncs.numcmp('2', 'abc')).toThrow();
  expect(() => builtinFuncs.numcmp('1')).toThrow();
  expect(() => builtinFuncs.numcmp()).toThrow();
});

test('builtinFuncs - add', () => {
  expect(builtinFuncs.add('1', '2')).toBe('3');
  expect(() => builtinFuncs.add('abc', '2')).toThrow();
  expect(() => builtinFuncs.add('2', 'abc')).toThrow();
  expect(() => builtinFuncs.add('1')).toThrow();
  expect(() => builtinFuncs.add()).toThrow();
});

test('builtinFuncs - sub', () => {
  expect(builtinFuncs.sub('3', '2')).toBe('1');
  expect(() => builtinFuncs.sub('abc', '2')).toThrow();
  expect(() => builtinFuncs.sub('2', 'abc')).toThrow();
  expect(() => builtinFuncs.sub('1')).toThrow();
  expect(() => builtinFuncs.sub()).toThrow();
});

test('builtinFuncs - mul', () => {
  expect(builtinFuncs.mul('2', '3')).toBe('6');
  expect(() => builtinFuncs.mul('abc', '2')).toThrow();
  expect(() => builtinFuncs.mul('2', 'abc')).toThrow();
  expect(() => builtinFuncs.mul('1')).toThrow();
  expect(() => builtinFuncs.mul()).toThrow();
});

test('builtinFuncs - div', () => {
  expect(builtinFuncs.div('6', '2')).toBe('3');
  expect(() => builtinFuncs.div('abc', '2')).toThrow();
  expect(() => builtinFuncs.div('2', 'abc')).toThrow();
  expect(() => builtinFuncs.div('1')).toThrow();
  expect(() => builtinFuncs.div()).toThrow();
});

test('builtinFuncs - mod', () => {
  expect(builtinFuncs.mod('7', '3')).toBe('1');
  expect(() => builtinFuncs.mod('abc', '2')).toThrow();
  expect(() => builtinFuncs.mod('2', 'abc')).toThrow();
  expect(() => builtinFuncs.mod('1')).toThrow();
  expect(() => builtinFuncs.mod()).toThrow();
});

test('builtinFuncs - random', () => {
  const result = builtinFuncs.random('1', '2', '3');
  expect(['1', '2', '3']).toContain(result);
  expect(() => builtinFuncs.random()).toThrow();
});

test('builtinFuncs - randomInt', () => {
  const result = builtinFuncs.randomInt('1', '10');
  expect(Number(result)).toBeGreaterThanOrEqual(1);
  expect(Number(result)).toBeLessThanOrEqual(10);
  expect(() => builtinFuncs.randomInt('1.2', '2')).toThrow();
  expect(() => builtinFuncs.randomInt('1', '2.2')).toThrow();
  expect(() => builtinFuncs.randomInt('5', '4')).toThrow();
  expect(() => builtinFuncs.randomInt('abc', '2')).toThrow();
  expect(() => builtinFuncs.randomInt('abc', '2')).toThrow();
  expect(() => builtinFuncs.randomInt('2', 'abc')).toThrow();
  expect(() => builtinFuncs.randomInt('1')).toThrow();
  expect(() => builtinFuncs.randomInt()).toThrow();
});

test('builtinFuncs - time', () => {
  expect(builtinFuncs.time()).toEqual(new Date().toLocaleTimeString());
});

test('builtinFuncs - date', () => {
  expect(builtinFuncs.date()).toEqual(new Date().toLocaleDateString());
});
