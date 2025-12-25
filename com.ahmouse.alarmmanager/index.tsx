import { registerRootComponent } from 'expo';

import App from './App';
import {PaperProvider } from 'react-native-paper';


export default function Main() {
  return (
    <PaperProvider>
      <App />
    </PaperProvider>
  );
}

// registerRootComponent calls AppRegistry.registerComponent('main', () => Main);
// It also ensures that whether you load the app in Expo Go or in a native build,
// the environment is set up appropriately
registerRootComponent(Main);
