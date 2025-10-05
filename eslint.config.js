// Flat ESLint config (ESLint v9+)
// Mirrors previous .eslintrc.json settings and .eslintignore patterns.

const globals = require('globals');

module.exports = [
  {
    files : ['data/**/*.js'],
    languageOptions : {
      ecmaVersion : 2020,
      sourceType : 'script',
      globals : {...globals.browser, WebSocket : 'readonly'}
    },
    rules : {
      'no-unused-vars' : ['warn', {args : 'none', ignoreRestSiblings : true}],
      'no-undef' : 'error',
      'no-var' : 'warn',
      'prefer-const' : ['warn', {destructuring : 'all'}],
      'semi' : ['error', 'always'],
      'quotes' : ['error', 'single', {avoidEscape : true}],
      'no-console' : 'off'
    }
  },
  {
    ignores :
    [
      '.pio/', 'build/', 'dist/', 'src/', 'lib/', 'include/', 'doc/', 'attic/',
      'test/', '*.png', '*.jpg', '*.webp', '*.pdf'
    ]
  }
];
