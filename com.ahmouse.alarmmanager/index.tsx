import { registerRootComponent } from 'expo';

import * as Theme from './material-theme.json';

import App from './App';
import {MD3LightTheme as DefaultTheme, PaperProvider } from 'react-native-paper';

const theme = {
  ...DefaultTheme,
  colors: Theme.dark
};

export default function Main() {
  return (
    <PaperProvider theme={theme}>
      <App />
    </PaperProvider>
  );
}

// registerRootComponent calls AppRegistry.registerComponent('main', () => Main);
// It also ensures that whether you load the app in Expo Go or in a native build,
// the environment is set up appropriately
registerRootComponent(Main);
