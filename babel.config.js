module.exports = {
  presets: ['module:react-native-builder-bob/babel-preset'],
  overrides: [
    {
      test: /node_modules[\\/]react-native[\\/]/,
      presets: ['module:@react-native/babel-preset'],
    },
  ],
};
