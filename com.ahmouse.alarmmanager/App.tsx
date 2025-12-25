import * as React from 'react';
import { StatusBar } from 'expo-status-bar';
import { StyleSheet, Text, View } from 'react-native';
import {AlarmList} from './components/alarm';
import { Appbar, FAB, Portal, useTheme } from 'react-native-paper';
import { createTimeObject } from './lib/time';

export default function App() {
  const theme = useTheme();

  const styles = StyleSheet.create({
    rootContainer: {
      width: '100%',
      height: '100%',
      backgroundColor: theme.colors.surface,
    },
    container: {
      flex: 1,
      backgroundColor: theme.colors.surface,
      alignItems: 'center',
      justifyContent: 'flex-start',
      marginTop: 20
    },
    appbar: {

    },
    addButton: {
      position: 'absolute',
      right: 0,
      bottom: 0,
      marginRight: 36,
      marginBottom: 96
    }
  });

  let alarms = [
    {
      id: '1',
      time: createTimeObject(10, 30)
    },
    {
      id: '2',
      time: createTimeObject(12, 23)
    },
  ];

  return (
    <View style={styles.rootContainer}>
      <StatusBar/>
      <Appbar.Header style={styles.appbar}>
        <Appbar.Content title="Alarms" />
        <Appbar.Action icon="menu" onPress={() => {}} />
      </Appbar.Header>

      <AlarmList items={alarms}/>
      <Portal>
        <FAB icon="plus" style={styles.addButton}/>
      </Portal>
      
    </View>
  );
}


