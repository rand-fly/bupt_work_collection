export const monarch = {
  // Set defaultToken to invalid to see what you do not tokenize yet
  // defaultToken: 'invalid',

  keywords: [
    'state',
    'enter',
    'case',
    'default',
    'silent',
    'say',
    'suggest',
    'delay',
    'goto',
    'exit',
    'let'
  ],

  // The main tokenizer for our languages
  tokenizer: {
    root: [
      // identifiers and keywords
      [
        /[a-z_$][\w$]*/,
        {
          cases: {
            '@keywords': 'keyword',
            '@default': 'identifier'
          }
        }
      ],

      // whitespace
      [/[ \t\r\n]+/, 'white'],
      [/#.*$/, 'comment'],

      // delimiters
      [/[()[\]]/, '@brackets'],

      // numbers
      [/\d*(\.\d*)+/, 'number.float'],
      [/\d+\.?/, 'number'],

      [/,/, 'delimiter'],

      // strings
      [/"([^"\\]|\\.)*$/, 'string.invalid'], // non-teminated string
      [/"/, { token: 'string.quote', bracket: '@open', next: '@string' }],

      // regexps
      [/\/([^/\\]|\\.)*$/, 'regexp.invalid'], // non-teminated regexp
      [/\//, { token: 'regexp.quote', bracket: '@open', next: '@regexp' }]
    ],

    string: [
      [/[^\\"]+/, 'string'],
      [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
    ],

    regexp: [
      [/[^\\/]+/, 'regexp'],
      [/\//, { token: 'regexp.quote', bracket: '@close', next: '@pop' }]
    ]
  }
};
