//
//  ReactiveObjC.h
//  ReactiveObjC
//
//  Created by Josh Abernathy on 3/5/12.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import <Foundation/Foundation>

//! Project version number for ReactiveObjC.
FOUNDATION_EXPORT double ReactiveObjCVersionNumber;

//! Project version string for ReactiveObjC.
FOUNDATION_EXPORT const unsigned char ReactiveObjCVersionString[];

#import "RACEXTKeyPathCoding.h"
#import "RACEXTScope.h"
#import "NSArray+RACSequenceAdditions"
#import "NSData+RACSupport"
#import "NSDictionary+RACSequenceAdditions"
#import "NSEnumerator+RACSequenceAdditions"
#import "NSFileHandle+RACSupport"
#import "NSNotificationCenter+RACSupport"
#import "NSObject+RACDeallocating"
#import "NSObject+RACLifting"
#import "NSObject+RACPropertySubscribing"
#import "NSObject+RACSelectorSignal"
#import "NSOrderedSet+RACSequenceAdditions"
#import "NSSet+RACSequenceAdditions"
#import "NSString+RACSequenceAdditions"
#import "NSString+RACSupport"
#import "NSIndexSet+RACSequenceAdditions"
#import "NSUserDefaults+RACSupport"
#import "RACBehaviorSubject"
#import "RACChannel"
#import "RACCommand"
#import "RACCompoundDisposable"
#import "RACDelegateProxy"
#import "RACDisposable"
#import "RACEvent"
#import "RACGroupedSignal"
#import "RACKVOChannel"
#import "RACMulticastConnection"
#import "RACQueueScheduler"
#import "RACQueueScheduler+Subclass"
#import "RACReplaySubject"
#import "RACScheduler"
#import "RACScheduler+Subclass"
#import "RACScopedDisposable"
#import "RACSequence"
#import "RACSerialDisposable"
#import "RACSignal+Operations"
#import "RACSignal"
#import "RACStream"
#import "RACSubject"
#import "RACSubscriber"
#import "RACSubscriptingAssignmentTrampoline"
#import "RACTargetQueueScheduler"
#import "RACTestScheduler"
#import "RACTuple"
#import "RACUnit"

#if TARGET_OS_WATCH
#elif TARGET_OS_IOS || TARGET_OS_TV
	#import "UIBarButtonItem+RACCommandSupport"
	#import "UIButton+RACCommandSupport"
	#import "UICollectionReusableView+RACSignalSupport"
	#import "UIControl+RACSignalSupport"
	#import "UIGestureRecognizer+RACSignalSupport"
	#import "UISegmentedControl+RACSignalSupport"
	#import "UITableViewCell+RACSignalSupport"
	#import "UITableViewHeaderFooterView+RACSignalSupport"
	#import "UITextField+RACSignalSupport"
	#import "UITextView+RACSignalSupport"

	#if TARGET_OS_IOS
		#import "NSURLConnection+RACSupport"
		#import "UIStepper+RACSignalSupport"
		#import "UIDatePicker+RACSignalSupport"
		#import "UIAlertView+RACSignalSupport"
		#import "UIActionSheet+RACSignalSupport"
		#import "MKAnnotationView+RACSignalSupport"
		#import "UIImagePickerController+RACSignalSupport"
		#import "UIRefreshControl+RACCommandSupport"
		#import "UISlider+RACSignalSupport"
		#import "UISwitch+RACSignalSupport"
	#endif
#elif TARGET_OS_MAC
	#import "NSControl+RACCommandSupport"
	#import "NSControl+RACTextSignalSupport"
	#import "NSObject+RACAppKitBindings"
	#import "NSText+RACSignalSupport"
	#import "NSURLConnection+RACSupport"
#endif
