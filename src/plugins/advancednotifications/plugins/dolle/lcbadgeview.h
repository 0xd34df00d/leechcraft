/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#import <AppKit/NSView.h>

@interface LCBadgeView : NSView
{
	NSArray* badges;
	NSArray* colors;
}

- (void)dealloc;
- (BOOL)displayBadges: (NSArray*)b andColors: (NSArray*)c;
- (void)drawRect: (NSRect)rect;
- (NSString*)elideString: (NSString*)s forWidth: (CGFloat)width outSize: (NSSize*)pSize;
- (int)maxBadges;
- (NSArray*)getGradientColorsForColor: (NSColor*)initialColor;
@end
