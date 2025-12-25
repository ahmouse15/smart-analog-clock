import * as React from 'react';
import { Button, Modal, Portal, Switch, Text, TouchableRipple, useTheme } from 'react-native-paper';
import { StyleSheet, View } from 'react-native';
import { FlatList } from 'react-native';
import { UUIDTypes, v4 as uuidv4 } from 'uuid';
import { useState } from 'react';

type Alarm = {
  id: string,
  time: Date
};

type AlarmProps = {
  alarm: Alarm
};

type AlarmListProps = {
  items: Alarm[]
};

type AlarmSettingsModalProps = AlarmProps & {
  visible: boolean,
  onPressClose: () => {}
};

export function AlarmListItem(props: AlarmProps) {
  const theme = useTheme();

  const styles = StyleSheet.create({
    rippleContainer: {
      height: "auto",
      width: '100%',
    },
    container: {
      height: "auto",
      width: '100%',
      backgroundColor: 'none',
      alignItems: 'center',
      flexDirection: 'row',
      justifyContent: 'space-between',
      padding: 10,
      pointerEvents: 'box-none'
    },
    leftContainer: {
    },
    rightContainer: {
    }
  });

  // Extract hours and minutes in locale-specific manner
  let timeStr = [props.alarm.time.getHours(), props.alarm.time.getMinutes()].join(':');

  return (
    <TouchableRipple style={styles.rippleContainer} disabled={false}>
      <View style={styles.container}>
        <View style={styles.leftContainer}>
          <Text variant="displayLarge">{timeStr}</Text>
          <Text variant="titleMedium">Schedule</Text>
        </View>
        <View style={styles.rightContainer}>
          <Switch/>
        </View>
      </View>
    </TouchableRipple>
  );
}

export function AlarmSettingsModal(props: AlarmSettingsModalProps) {
  const theme = useTheme();

  const styles = StyleSheet.create({
    container: {
      flex: 1,
      backgroundColor: theme.colors.primaryContainer,
      alignItems: 'center',
      justifyContent: 'flex-start',
      marginTop: 20
    }
  });

  return (
      <View style={styles.container}>
        <Portal>
          <Modal visible={props.visible}>
            <Text>Placeholder</Text>
            <Button onPress={props.onPressClose}>Close</Button>
          </Modal>
        </Portal>
      </View>
  );
}

export function AlarmList(props: AlarmListProps) {
  const theme = useTheme();

  //TODO: for x in

  const styles = StyleSheet.create({
    container: {
      flex: 1,
      backgroundColor: theme.colors.primaryContainer,
      alignItems: 'center',
      justifyContent: 'flex-start',
      marginTop: 20
    }
  });

  return (
      <View style={styles.container}>
        <FlatList 
          data={props.items}
          renderItem={({item}) => <AlarmListItem alarm={item} />}
        />
      </View>
  );
}