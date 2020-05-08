//
//  Person.h
//  ReactiveObjCStudy
//
//  Created by Harwin on 2020/5/8.
//  Copyright © 2020 WoQi. All rights reserved.
//

#import <Foundation/Foundation.h>

//NS_ASSUME_NONNULL_BEGIN

@interface Language : NSObject;

@end

@interface Java : Language;

@end

@interface iOS : Language;

@end

@interface Person<__covariant ValueType> : NSObject

@property (nonatomic, strong) ValueType language;

@end

//NS_ASSUME_NONNULL_END
