import * as React from 'react';
import { StatusBar } from 'expo-status-bar';
import { StyleSheet, Text, View } from 'react-native';
import {AlarmListItem} from './components/alarm';
import { Appbar } from 'react-native-paper';

export default function App() {
  let time = new Date(1980, 1, 5, 10, 30);
  return (
    <View style={styles.container}>

      <StatusBar
        backgroundColor="red"
      />

      <Appbar.Header style={styles.appbar}>
        <Appbar.Content title="Alarms" />
        <Appbar.Action icon="dots" onPress={() => {}} />
      </Appbar.Header>

      <AlarmListItem time={time}/>

    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#fff',
    alignItems: 'center',
    justifyContent: 'flex-start',
    marginTop: 20
  },
  appbar: {

  }
});
