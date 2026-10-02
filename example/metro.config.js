const { getDefaultConfig, mergeConfig } = require('@react-native/metro-config');
const fs = require('fs');
const path = require('node:path');
const escape = require('escape-string-regexp');
const pack = require('../package.json');

const root = path.resolve(__dirname, '..');
const exampleModules = path.join(__dirname, 'node_modules');
const modules = Object.keys({ ...pack.peerDependencies });

// Metro's file map matches paths with either separator. A pattern built with
// path.join() only matches backslashes on Windows, so the root copies of
// react and react-native were never excluded.
function pathPattern(absolutePath, suffix = '[/\\\\].*') {
  const parts = absolutePath.split(/[/\\]/).filter(Boolean);
  const leading = absolutePath.startsWith('/') ? '[/\\\\]' : '';
  return new RegExp(
    `^${leading}${parts.map((part) => escape(part)).join('[/\\\\]')}${suffix}`
  );
}

function blockRootModule(moduleName) {
  return pathPattern(path.join(root, 'node_modules', moduleName));
}

const rnwPath = fs.realpathSync(
  path.resolve(require.resolve('react-native-windows/package.json'), '..')
);

const defaultConfig = getDefaultConfig(__dirname);
const defaultBlockList = defaultConfig.resolver?.blockList;
const defaultPatterns = Array.isArray(defaultBlockList)
  ? defaultBlockList
  : defaultBlockList
    ? [defaultBlockList]
    : [];

// `react-native start` installs this before loading the config. On Windows it
// rewrites `react-native` to `react-native-windows`, which is where
// ReactDevToolsSettingsManager.windows.js lives. Replacing it drops that rewrite.
const upstreamResolveRequest = defaultConfig.resolver?.resolveRequest;

/**
 * Metro configuration
 * https://facebook.github.io/metro/docs/configuration
 *
 * @type {import('metro-config').MetroConfig}
 */
const config = {
  watchFolders: [root],

  // Peer dependencies must be the example's copies. Metro only reads
  // resolver.blockList (not blocklist). extraNodeModules is a fallback after
  // hierarchical lookup, so root copies also have to be blocked.
  resolver: {
    blockList: [
      ...defaultPatterns,
      ...modules.map((moduleName) => blockRootModule(moduleName)),
      pathPattern(path.resolve(__dirname, 'windows'), '.*'),
      pathPattern(path.join(rnwPath, 'build')),
      pathPattern(path.join(rnwPath, 'target')),
      /.*\.ProjectImports\.zip/,
    ],

    extraNodeModules: {
      ...modules.reduce((acc, name) => {
        acc[name] = path.join(exampleModules, name);
        return acc;
      }, {}),
      [pack.name]: root,
    },

    resolveRequest: (context, moduleName, platform) => {
      if (moduleName === pack.name) {
        return context.resolveRequest(
          context,
          path.join(root, 'src', 'index.tsx'),
          platform
        );
      }
      if (typeof upstreamResolveRequest === 'function') {
        return upstreamResolveRequest(context, moduleName, platform);
      }
      return context.resolveRequest(context, moduleName, platform);
    },
  },

  transformer: {
    getTransformOptions: async () => ({
      transform: {
        experimentalImportSupport: false,
        inlineRequires: true,
      },
    }),
  },
};

module.exports = mergeConfig(defaultConfig, config);
