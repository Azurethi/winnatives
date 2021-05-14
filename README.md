# winnatives
 Gives node.js access to some windows API calls

## building

run the following:
```
node-gyp configure
node-gyp build
```
and you're good to go!

## using

To access the natives directly
```js
const WN = require('./build/Release/winnatives');
```

To try out the example wrapper script
```js
const wrapper = require('./ssc_wrapper');
```
